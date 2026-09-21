// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
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
 */

#include "RemoteTextBrowser.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextDocument>

RemoteTextBrowser::RemoteTextBrowser(QWidget* parent)
    : QTextBrowser(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &RemoteTextBrowser::imageDownloadFinished);

    // Debounce timer: when an image finishes downloading, wait briefly
    // for more images to finish before re-rendering the HTML once.
    m_renderTimer.setSingleShot(true);
    m_renderTimer.setInterval(150);
    connect(&m_renderTimer, &QTimer::timeout, this, &RemoteTextBrowser::reRender);
}

QVariant RemoteTextBrowser::loadResource(int type, const QUrl& name)
{
    if (type == QTextDocument::ImageResource && name.scheme().startsWith("http")) {
        // Check our persistent image cache first
        if (m_downloadedImages.contains(name))
            return m_downloadedImages.value(name);

        // Don't re-request images that are already downloading or have failed
        if (m_pendingImages.contains(name) || m_failedImages.contains(name))
            return {};

        // Start async download
        m_pendingImages.insert(name);

        QNetworkRequest request(name);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        QNetworkReply* reply = m_networkManager->get(request);
        // Track the original URL ourselves — guaranteed to survive redirects
        m_replyToOriginalUrl.insert(reply, name);

        return {};  // Return empty for now; will re-render when downloads complete
    }

    return QTextBrowser::loadResource(type, name);
}

void RemoteTextBrowser::setHtml(const QString& text)
{
    m_currentHtml = text;
    m_renderTimer.stop();

    // Clear caches when content changes entirely
    m_pendingImages.clear();
    m_failedImages.clear();
    m_downloadedImages.clear();

    QTextBrowser::setHtml(text);
}

void RemoteTextBrowser::clear()
{
    m_currentHtml.clear();
    m_renderTimer.stop();
    m_pendingImages.clear();
    m_failedImages.clear();
    m_downloadedImages.clear();
    QTextBrowser::clear();
}

void RemoteTextBrowser::imageDownloadFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    // Retrieve the original (pre-redirect) URL from our tracking map
    QUrl originalUrl = m_replyToOriginalUrl.take(reply);
    if (!originalUrl.isValid())
        originalUrl = reply->url();

    m_pendingImages.remove(originalUrl);

    // If content has been cleared or changed since this request started, ignore
    if (m_currentHtml.isEmpty())
        return;

    if (reply->error() != QNetworkReply::NoError) {
        m_failedImages.insert(originalUrl);
        return;
    }

    QByteArray data = reply->readAll();
    QImage image;
    if (!image.loadFromData(data)) {
        m_failedImages.insert(originalUrl);
        return;
    }

    // Scale down large images to fit the viewport width
    int maxWidth = viewport()->width() - 10;
    if (image.width() > maxWidth)
        image = image.scaledToWidth(maxWidth, Qt::SmoothTransformation);

    m_downloadedImages.insert(originalUrl, image);

    // If all pending images are done, re-render immediately.
    // Otherwise, debounce — wait 150ms for more images to finish.
    if (m_pendingImages.isEmpty()) {
        m_renderTimer.stop();
        reRender();
    } else {
        m_renderTimer.start();
    }
}

void RemoteTextBrowser::reRender()
{
    if (m_currentHtml.isEmpty())
        return;

    // Re-set the original HTML. loadResource() will return cached images
    // from m_downloadedImages immediately, so this is fast.
    QTextBrowser::setHtml(m_currentHtml);
}
