// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 Lenny McLennington <lenny@sneed.church>
 *  Copyright (C) 2022 Swirl <swurl@swurl.xyz>
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

#include "PasteUpload.h"
#include "BuildConfig.h"
#include "Application.h"

#include <QDebug>
#include <QFile>

std::array<PasteUpload::PasteTypeInfo, 4> PasteUpload::PasteTypes = {
    {{"0x0.st", "https://0x0.st", ""},
     {"hastebin", "https://hst.sh", "/documents"},
     {"paste.gg", "https://paste.gg", "/api/v1/pastes"},
     {"mclo.gs", "https://api.mclo.gs", "/1/log"}}};

PasteUpload::PasteUpload(QWidget *window, const QString& text, QString baseUrl, PasteType pasteType) : m_window(window), m_baseUrl(baseUrl), m_pasteType(pasteType), m_text(text.toUtf8())
{
    if (m_baseUrl == "")
        m_baseUrl = PasteTypes.at(pasteType).defaultBase;

    // HACK: Paste's docs say the standard API path is at /api/<version> but the official instance paste.gg doesn't follow that??
    if (pasteType == PasteGG && m_baseUrl == PasteTypes.at(pasteType).defaultBase)
        m_uploadUrl = "https://api.paste.gg/v1/pastes";
    else
        m_uploadUrl = m_baseUrl + PasteTypes.at(pasteType).endpointPath;
}

PasteUpload::~PasteUpload() = default;

void PasteUpload::executeTask()
{
    QNetworkRequest request{QUrl(m_uploadUrl)};
    QNetworkReply *rep{};

    request.setHeader(QNetworkRequest::UserAgentHeader, APPLICATION->getUserAgentUncached().toUtf8());

    switch (m_pasteType) {
    case NullPointer: {
        QHttpMultiPart *multiPart =
          new QHttpMultiPart{QHttpMultiPart::FormDataType};

        QHttpPart filePart;
        filePart.setBody(m_text);
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                         R"(form-data; name="file"; filename="log.txt")");
        multiPart->append(filePart);

        rep = APPLICATION->network()->post(request, multiPart);
        multiPart->setParent(rep);

        break;
    }
    case Hastebin: {
        request.setHeader(QNetworkRequest::UserAgentHeader, APPLICATION->getUserAgentUncached().toUtf8());
        rep = APPLICATION->network()->post(request, m_text);
        break;
    }
    case Mclogs: {
        QUrlQuery postData;
        postData.addQueryItem("content", m_text);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        rep = APPLICATION->network()->post(request, postData.toString().toUtf8());
        break;
    }
    case PasteGG: {
        nlohmann::json obj;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        obj["expires"] = QDateTime::currentDateTimeUtc().addDays(100).toString(Qt::DateFormat::ISODate).toStdString();

        nlohmann::json files;
        nlohmann::json logFileInfo;
        nlohmann::json logFileContentInfo;
        logFileContentInfo["format"] = "text";
        logFileContentInfo["value"] = QString::fromUtf8(m_text).toStdString();
        logFileInfo["name"] = "log.txt";
        logFileInfo["content"] = logFileContentInfo;
        files.push_back(logFileInfo);

        obj["files"] = files;

        QString json = obj.dump().c_str();
        rep = APPLICATION->network()->post(request, json.toUtf8());
        break;
    }
    }

    connect(rep, &QNetworkReply::uploadProgress, this, &Task::setProgress);
    connect(rep, &QNetworkReply::finished, this, &PasteUpload::downloadFinished);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(rep, &QNetworkReply::errorOccurred, this, &PasteUpload::downloadError);
#else
    connect(rep, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &PasteUpload::downloadError);
#endif


    m_reply = std::shared_ptr<QNetworkReply>(rep);

    setStatus(tr("Uploading to %1").arg(m_uploadUrl));
}

void PasteUpload::downloadError(QNetworkReply::NetworkError error)
{
    // error happened during download.
    qCritical() << "Network error: " << error;
    emitFailed(m_reply->errorString());
}

void PasteUpload::downloadFinished()
{
    QByteArray data = m_reply->readAll();
    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (m_reply->error() != QNetworkReply::NetworkError::NoError)
    {
        emitFailed(tr("Network error: %1").arg(m_reply->errorString()));
        m_reply.reset();
        return;
    }
    else if (statusCode != 200 && statusCode != 201)
    {
        QString reasonPhrase = m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        emitFailed(tr("Error: %1 returned unexpected status code %2 %3").arg(m_uploadUrl).arg(statusCode).arg(reasonPhrase));
        qCritical() << m_uploadUrl << " returned unexpected status code " << statusCode << " with body: " << data;
        m_reply.reset();
        return;
    }

    switch (m_pasteType)
    {
    case NullPointer:
        m_pasteLink = QString::fromUtf8(data).trimmed();
        break;
    case Hastebin: {
        nlohmann::json jsonObj = nlohmann::json::parse(data.constData(), data.constData() + data.size());

        const nlohmann::json& temp = jsonObj.value("key", nlohmann::json());
        if (temp.is_string())
        {
            QString key = temp.get<std::string>().c_str();
            m_pasteLink = m_baseUrl + "/" + key;
        }
        else
        {
            emitFailed(tr("Error: %1 returned a malformed response body").arg(m_uploadUrl));
            qCritical() << m_uploadUrl << " returned malformed response body: " << data;
            return;
        }
        break;
    }
    case Mclogs: {
        nlohmann::json jsonObj = nlohmann::json::parse(data.constData(), data.constData() + data.size());

        const nlohmann::json& temp = jsonObj.value("success", nlohmann::json());
        if (temp.is_boolean())
        {
            bool success = temp.get<bool>();
            if (success)
            {
                m_pasteLink = jsonObj["url"].get<std::string>().c_str();
            }
            else
            {
                QString error = jsonObj["error"].get<std::string>().c_str();
                emitFailed(tr("Error: %1 returned an error: %2").arg(m_uploadUrl, error));
                qCritical() << m_uploadUrl << " returned error: " << error;
                qCritical() << "Response body: " << data;
                return;
            }
        }
        else
        {
            emitFailed(tr("Error: %1 returned a malformed response body").arg(m_uploadUrl));
            qCritical() << m_uploadUrl << " returned malformed response body: " << data;
            return;
        }
        break;
    }
    case PasteGG:
        nlohmann::json jsonObj = nlohmann::json::parse(data.constData(), data.constData() + data.size());

        const nlohmann::json& temp = jsonObj.value("status", nlohmann::json());
        if (temp.is_string())
        {
            QString status = temp.get<std::string>().c_str();
            if (status == "success")
            {
                m_pasteLink = m_baseUrl + "/p/anonymous/" + jsonObj["result"].get<std::string>().c_str();
            }
            else
            {
                QString error = jsonObj["error"].get<std::string>().c_str();
                QString message = (jsonObj.contains("message") && jsonObj["message"].is_string()) ? jsonObj["message"].get<std::string>().c_str() : "none";
                emitFailed(tr("Error: %1 returned an error code: %2\nError message: %3").arg(m_uploadUrl, error, message));
                qCritical() << m_uploadUrl << " returned error: " << error;
                qCritical() << "Error message: " << message;
                qCritical() << "Response body: " << data;
                return;
            }
        }
        else
        {
            emitFailed(tr("Error: %1 returned a malformed response body").arg(m_uploadUrl));
            qCritical() << m_uploadUrl << " returned malformed response body: " << data;
            return;
        }
        break;
    }
    emitSucceeded();
}
