#include "RemoteTextBrowser.h"

#include "Application.h"
#include "DesktopServices.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QImageReader>
#include <QPointer>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextImageFormat>

#include <QSize>
#include <QTimer>

namespace {

// replaces the broken image icon before load
QImage placeholderImage() {
    static QImage img = [] {
        QImage i(1, 1, QImage::Format_ARGB32);
        i.fill(Qt::transparent);
        return i;
    }();
    return img;
}

} // namespace

RemoteTextBrowser::RemoteTextBrowser(QWidget* parent) : QTextBrowser(parent) {
    document()->setDefaultStyleSheet(QStringLiteral("code { white-space: pre-wrap; }"));
    document()->setUndoRedoEnabled(false);

    setOpenLinks(false);
    connect(this, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
        if (url.scheme() == "http" || url.scheme() == "https" || url.scheme() == "mailto")
            DesktopServices::openUrl(url);
        else if (url.isRelative() && url.path().isEmpty() && url.hasFragment())
            scrollToAnchor(url.fragment());
    });
}

QVariant RemoteTextBrowser::loadResource(int type, const QUrl& url) {
    if (type != QTextDocument::ImageResource || (url.scheme() != "http" && url.scheme() != "https"))
        return {};

    // load an img to the doc
    QPointer<QTextDocument> requestingDoc = document();
    auto load = [this, url, requestingDoc](const QImage& img) {
        m_loading.remove(url);
        if (img.isNull()) {
            m_failed.insert(url);

            // don't render this image at all
            if (requestingDoc && document() == requestingDoc)
                rerender(requestingDoc, url, QSize(0, 0));

            return;
        }

        // another doc was navigated to during download
        if (!requestingDoc || document() != requestingDoc)
            return;

        QTimer::singleShot(0, this, [this, url, img, doc = QPointer<QTextDocument>(document())] {
            if (!doc || document() != doc)
                return;
            doc->addResource(QTextDocument::ImageResource, url, img);
            rerender(doc, url, img.size());
        });

        viewport()->update();
    };

    // do not attempt reload
    if (m_loading.contains(url) || m_failed.contains(url))
        return placeholderImage();

    const auto key = QCryptographicHash::hash(url.toString(QUrl::FullyEncoded).toUtf8(),
                                              QCryptographicHash::Sha1)
                         .toHex();
    auto entry = APPLICATION->metacache()->resolveEntry("images", key);

    // disk cache
    if (!entry->isStale() && QFile::exists(entry->getFullPath())) {
        QImage img = loadScaled(entry->getFullPath());
        if (!img.isNull()) {
            load(img);
            return placeholderImage();
        }
    }

    auto job = new NetJob(QStringLiteral("Download Image %1").arg(url.fileName()),
                          APPLICATION->network(), this);

    job->addNetAction(Net::Download::makeCached(url, entry));

    connect(job, &NetJob::succeeded, this, [url, entry, load, this]() {
        load(loadScaled(entry->getFullPath()));
    });

    connect(job, &NetJob::failed, this, [url, entry, load](const QString& reason) {
        qDebug() << "Failed to load image from" << url << "at" << entry->getFullPath()
                 << "because:" << reason;

        load(QImage{});
    });

    connect(job, &NetJob::finished, job, [this, job]() {
        m_jobs.removeAll(job);
        job->deleteLater();
    });

    m_loading.insert(url);
    m_jobs.append(job);
    job->start();

    return placeholderImage();
}

void RemoteTextBrowser::setHtml(const QString& text) {
    // abort remaining jobs
    const auto jobs = m_jobs;
    m_jobs.clear();

    for (auto job : jobs) {
        if (job && job->isRunning())
            job->abort();
    }

    m_loading.clear();
    m_failed.clear();
    QTextBrowser::setHtml(text);

    auto doc = document();
    if (!doc)
        return;

    // collect image urls
    const auto frags = collectFragments(doc);
    for (const auto& frag : frags) {
        if (frag.url.scheme() != "http" && frag.url.scheme() != "https")
            continue;

        // prevent the images from rendering for now
        // this prevents e.g. gigantic images from taking up a bunch of blank space
        auto fmt = frag.fmt;
        fmt.setProperty(AuthorSize, QSizeF(qMax(qreal(0), fmt.width()), qMax(qreal(0), fmt.height())));
        applyFormat({0, 0}, true, doc, frag.pos, fmt);

        loadResource(QTextDocument::ImageResource, frag.url);
    }
}

void RemoteTextBrowser::rerender(QTextDocument* doc, const QUrl& url, const QSize& imageSize) {
    // custom sized images in html need special handling
    const auto frags = collectFragments(doc);
    for (const auto& frag : frags) {
        if (frag.url != url)
            continue;

        QTextImageFormat fmt = frag.fmt;

        // forcefully downsize giant images
        if (fmt.isValid() && (fmt.width() > imageSize.width() || fmt.property(Marked).isValid())) {
            applyFormat(fittedSize(imageSize, fmt.property(AuthorSize).toSizeF()), {}, doc, frag.pos, fmt);
        } else {
            doc->markContentsDirty(frag.pos, 1);
        }
    }
}

QSize RemoteTextBrowser::fittedSize(const QSize& imageSize, const QSizeF& authorSize) const {
    if (imageSize.isEmpty())
        return {0, 0};

    QSizeF size = imageSize;
    if (authorSize.width() > 0 && authorSize.height() > 0)
        size = authorSize;
    else if (authorSize.width() > 0)
        size = {authorSize.width(), authorSize.width() * imageSize.height() / imageSize.width()};
    else if (authorSize.height() > 0)
        size = {authorSize.height() * imageSize.width() / imageSize.height(), authorSize.height()};

    const int maxWidth = viewport()->width() - 16;
    if (maxWidth >= 64 && size.width() > maxWidth)
        size *= maxWidth / size.width();

    return size.toSize();
}

void RemoteTextBrowser::applyFormat(const QSize& size, const QVariant& mark, QTextDocument* doc,
                                    int pos, QTextImageFormat fmt) {
    fmt.setWidth(size.width());
    fmt.setHeight(size.height());
    fmt.setProperty(Marked, mark);

    QTextCursor cursor(doc);
    cursor.setPosition(pos);
    cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
    cursor.mergeCharFormat(fmt);

    doc->markContentsDirty(pos, 1);
}

// faster decode directly from disk
QImage RemoteTextBrowser::loadScaled(const QString& path) {
    QImageReader reader(path);
    reader.setAutoTransform(true);

    if (!reader.canRead())
        return {};

    const int viewportWidth = viewport() ? viewport()->width() : 0;
    const auto w = viewportWidth - 16;

    if (w >= 64) {
        // even scale
        const QSize s = reader.size();

        if (s.isValid() && s.width() > w) {
            const auto h = qMax(1, s.height() * w / s.width());
            reader.setScaledSize({w, h});
        }
    }

    return reader.read();
}

QList<RemoteTextBrowser::ImageFragment> RemoteTextBrowser::collectFragments(
    const QTextDocument* doc) {
    QList<RemoteTextBrowser::ImageFragment> fragments;
    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        for (auto it = block.begin(); !it.atEnd(); ++it) {
            // check if its an image
            const auto frag = it.fragment();
            if (!frag.isValid() || !frag.charFormat().isImageFormat())
                continue;

            const auto fmt = frag.charFormat().toImageFormat();
            fragments.append({frag.position(), QUrl(fmt.name()), fmt});
        }
    }

    return fragments;
}
