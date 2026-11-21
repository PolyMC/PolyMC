#include "LogoCache.h"

#include "Application.h"
#include "net/NetJob.h"
#include "net/Download.h"
#include "net/HttpMetaCache.h"

#include <QUrl>

#include <functional>
#include <utility>

namespace
{
QString entryName(const QString& name)
{
    return QString("logos/%1").arg(name.section(".", 0, 0));
}
}  // namespace

LogoCache::LogoCache(QString base, QString job, QString placeholder, Skip skip,
                     QObject* parent)
    : QObject(parent),
      m_base(std::move(base)),
      m_job(std::move(job)),
      m_placeholder(std::move(placeholder)),
      m_skip(skip)
{
}

QIcon LogoCache::icon(const QString& name, const QString& url)
{
    if (m_icons.contains(name))
    {
        return m_icons.value(name);
    }
    request(name, url);
    return APPLICATION->getThemedIcon(m_placeholder);
}

QString LogoCache::path(const QString& name, const QString& url)
{
    if (m_icons.contains(name))
    {
        return APPLICATION->metacache()
            ->resolveEntry(m_base, entryName(name))
            ->getFullPath();
    }
    request(name, url);
    return {};
}

bool LogoCache::skips(const QString& name, const QString& url) const
{
    return (m_skip == Skip::EmptyUrl && url.isEmpty()) ||
           (m_skip == Skip::NullName && name == "null");
}

void LogoCache::request(const QString& name, const QString& url)
{
    if (m_loading.contains(name) || m_failed.contains(name) || skips(name, url))
    {
        return;
    }

    MetaEntryPtr entry =
        APPLICATION->metacache()->resolveEntry(m_base, entryName(name));
    auto* job =
        new NetJob(QString("%1 %2").arg(m_job, name), APPLICATION->network());
    job->addNetAction(Net::Download::makeCached(QUrl(url), entry));

    auto file = entry->getFullPath();
    // succeeded, not finished: finished also comes after a failure, which would
    // store an empty logo
    connect(job, &NetJob::succeeded, this,
            std::bind(&LogoCache::onDownloaded, this, name, file));
    connect(job, &NetJob::failed, this,
            std::bind(&LogoCache::onFailed, this, name));
    connect(job, &NetJob::finished, job, &QObject::deleteLater);

    job->start();

    m_loading.append(name);
}

void LogoCache::onDownloaded(const QString& name, const QString& file)
{
    m_loading.removeAll(name);
    m_icons.insert(name, QIcon(file));
    emit logoLoaded(name);
}

void LogoCache::onFailed(const QString& name)
{
    m_failed.append(name);
    m_loading.removeAll(name);
}
