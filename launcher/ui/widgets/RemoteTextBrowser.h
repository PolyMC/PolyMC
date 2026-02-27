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

#pragma once

#include <QTextBrowser>
#include <QHash>
#include <QSet>
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;

/**
 * A QTextBrowser subclass that can load remote images (http/https).
 *
 * The standard QTextBrowser only loads local resources. This subclass
 * intercepts image resource requests for remote URLs, downloads them
 * asynchronously, caches them in the document, and re-renders once
 * all images are ready (or after a short debounce timeout).
 */
class RemoteTextBrowser : public QTextBrowser {
    Q_OBJECT

   public:
    explicit RemoteTextBrowser(QWidget* parent = nullptr);

    /// Set HTML content. Also resets image caches.
    void setHtml(const QString& text);

    /// Clear content and reset state (mirrors QTextEdit::clear).
    void clear();

   protected:
    QVariant loadResource(int type, const QUrl& name) override;

   private slots:
    void imageDownloadFinished(QNetworkReply* reply);
    void reRender();

   private:
    QNetworkAccessManager* m_networkManager;
    QTimer m_renderTimer;
    QSet<QUrl> m_pendingImages;
    QSet<QUrl> m_failedImages;
    QHash<QUrl, QImage> m_downloadedImages;
    /// Maps in-flight QNetworkReply* → original request URL (before redirects)
    QHash<QNetworkReply*, QUrl> m_replyToOriginalUrl;
    QString m_currentHtml;
};
