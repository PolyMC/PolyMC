// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "Upload.h"

#include <utility>
#include "ByteArraySink.h"
#include "BuildConfig.h"
#include "Application.h"

namespace Net {

    bool Upload::abort()
    {
        if (m_reply) {
            m_reply->abort();
        } else {
            m_state = State::AbortedByUser;
        }
        return true;
    }

    void Upload::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
        setProgress(bytesReceived, bytesTotal);
    }

    void Upload::downloadError(QNetworkReply::NetworkError error) {
        if (error == QNetworkReply::OperationCanceledError) {
            qCritical() << "Aborted " << m_url.toString();
            m_state = State::AbortedByUser;
        } else {
            // error happened during download.
            qCritical() << "Failed " << m_url.toString() << " with reason " << error;
            m_state = State::Failed;
        }
    }

    void Upload::sslErrors(const QList<QSslError> &errors) {
        int i = 1;
        for (const auto& error : errors) {
            qCritical() << "Upload" << m_url.toString() << "SSL Error #" << i << " : " << error.errorString();
            auto cert = error.certificate();
            qCritical() << "Certificate in question:\n" << cert.toText();
            i++;
        }
    }

    bool Upload::handleRedirect()
    {
        QUrl redirect = m_reply->header(QNetworkRequest::LocationHeader).toUrl();
        if (!redirect.isValid()) {
            if (!m_reply->hasRawHeader("Location")) {
                // no redirect -> it's fine to continue
                return false;
            }
            // there is a Location header, but it's not correct. we need to apply some workarounds...
            QByteArray redirectBA = m_reply->rawHeader("Location");
            if (redirectBA.size() == 0) {
                // empty, yet present redirect header? WTF?
                return false;
            }
            QString redirectStr = QString::fromUtf8(redirectBA);

            if (redirectStr.startsWith("//")) {
                /*
                 * IF the URL begins with //, we need to insert the URL scheme.
                 * See: https://bugreports.qt.io/browse/QTBUG-41061
                 * See: http://tools.ietf.org/html/rfc3986#section-4.2
                 */
                redirectStr = m_reply->url().scheme() + ":" + redirectStr;
            } else if (redirectStr.startsWith("/")) {
                /*
                 * IF the URL begins with /, we need to process it as a relative URL
                 */
                auto url = m_reply->url();
                url.setPath(redirectStr, QUrl::TolerantMode);
                redirectStr = url.toString();
            }

            /*
             * Next, make sure the URL is parsed in tolerant mode. Qt doesn't parse the location header in tolerant mode, which causes issues.
             * FIXME: report Qt bug for this
             */
            redirect = QUrl(redirectStr, QUrl::TolerantMode);
            if (!redirect.isValid()) {
                qWarning() << "Failed to parse redirect URL:" << redirectStr;
                downloadError(QNetworkReply::ProtocolFailure);
                return false;
            }
            qDebug() << "Fixed location header:" << redirect;
        } else {
            qDebug() << "Location header:" << redirect;
        }

        m_url = QUrl(redirect.toString());
        qDebug() << "Following redirect to " << m_url.toString();
        startAction(m_network);
        return true;
    }

    void Upload::downloadFinished() {
        // handle HTTP redirection first
        // very unlikely for post requests, still can happen
        if (handleRedirect()) {
            qDebug() << "Upload redirected:" << m_url.toString();
            return;
        }

        // if the download failed before this point ...
        if (m_state == State::Succeeded) {
            qDebug() << "Upload failed but we are allowed to proceed:" << m_url.toString();
            m_sink->abort();
            m_reply.reset();
            emit succeeded();
            return;
        } else if (m_state == State::Failed) {
            qDebug() << "Upload failed in previous step:" << m_url.toString();
            m_sink->abort();
            m_reply.reset();
            emit failed("");
            return;
        } else if (m_state == State::AbortedByUser) {
            qDebug() << "Upload aborted in previous step:" << m_url.toString();
            m_sink->abort();
            m_reply.reset();
            emit aborted();
            return;
        }

        // make sure we got all the remaining data, if any
        auto data = m_reply->readAll();
        if (data.size()) {
            qDebug() << "Writing extra" << data.size() << "bytes";
            m_state = m_sink->write(data);
        }

        // otherwise, finalize the whole graph
        m_state = m_sink->finalize(*m_reply.get());
        if (m_state != State::Succeeded) {
            qDebug() << "Upload failed to finalize:" << m_url.toString();
            m_sink->abort();
            m_reply.reset();
            emit failed("");
            return;
        }
        m_reply.reset();
        qDebug() << "Upload succeeded:" << m_url.toString();
        emit succeeded();
    }

    void Upload::downloadReadyRead() {
        if (m_state == State::Running) {
            auto data = m_reply->readAll();
            m_state = m_sink->write(data);
        }
    }

    void Upload::executeTask() {
        setStatus(tr("Uploading %1").arg(m_url.toString()));

        if (m_state == State::AbortedByUser) {
            qWarning() << "Attempt to start an aborted Upload:" << m_url.toString();
            emit aborted();
            return;
        }
        QNetworkRequest request(m_url);
        m_state = m_sink->init(request);
        switch (m_state) {
            case State::Succeeded:
                emitSucceeded();
                qDebug() << "Upload cache hit " << m_url.toString();
                return;
            case State::Running:
                qDebug() << "Uploading " << m_url.toString();
                break;
            case State::Inactive:
            case State::Failed:
                emitFailed("");
                return;
            case State::AbortedByUser:
                emitAborted();
                return;
        }

        request.setHeader(QNetworkRequest::UserAgentHeader, APPLICATION->getUserAgent().toUtf8());
        if (APPLICATION->currentCapabilities() & Application::SupportsFlame
                && request.url().host().contains("api.curseforge.com")) {
            request.setRawHeader("x-api-key", APPLICATION->getFlameAPIKey().toUtf8());
        }
        //TODO other types of post requests ?
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply* rep = m_network->post(request, m_post_data);

        m_reply.reset(rep);
        connect(rep, SIGNAL(downloadProgress(qint64, qint64)), SLOT(downloadProgress(qint64, qint64)));
        connect(rep, SIGNAL(finished()), SLOT(downloadFinished()));
        connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(downloadError(QNetworkReply::NetworkError)));
        connect(rep, &QNetworkReply::sslErrors, this, &Upload::sslErrors);
        connect(rep, &QNetworkReply::readyRead, this, &Upload::downloadReadyRead);
    }

    Upload::Ptr Upload::makeByteArray(QUrl url, QByteArray *output, QByteArray m_post_data) {
        auto* up = new Upload();
        up->m_url = std::move(url);
        up->m_sink.reset(new ByteArraySink(output));
        up->m_post_data = std::move(m_post_data);
        return up;
    }
} // Net
