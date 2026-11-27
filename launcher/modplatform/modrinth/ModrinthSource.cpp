// SPDX-License-Identifier: GPL-3.0-only

#include "ModrinthSource.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QRegularExpression>
#include <QSet>
#include <QSize>
#include <QUrl>

#include <utility>

#include "Application.h"
#include "BuildConfig.h"
#include "FileSystem.h"
#include "net/Download.h"
#include "net/HttpMetaCache.h"
#include "net/NetJob.h"
#include "settings/SettingsObject.h"

namespace Modrinth
{

QString Source::displayName() const
{
    if (!name.isEmpty())
    {
        return name;
    }
    if (isOfficial())
    {
        return "Modrinth";
    }
    return QUrl(api_url).host();
}

QString Source::websiteRoot() const
{
    if (isOfficial())
    {
        return "https://modrinth.com";
    }
    if (!website_url.isEmpty())
    {
        return normalizeApiUrl(website_url);
    }

    QUrl api(api_url);
    return api
        .adjusted(QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment |
                  QUrl::RemoveUserInfo)
        .toString();
}

QString Source::key() const
{
    if (isOfficial())
    {
        return "modrinth";
    }

    // No dots, cache entry names are cut at the first one
    auto hash =
        QCryptographicHash::hash(api_url.toUtf8(), QCryptographicHash::Sha1);
    return "modrinth-" + QString::fromLatin1(hash.toHex().left(12));
}

QString Source::cacheName(const QString& id) const
{
    if (isOfficial())
    {
        return id;
    }
    return key() + "-" + id;
}

QString normalizeApiUrl(const QString& url)
{
    auto normalized = url.trimmed();
    while (normalized.endsWith('/'))
    {
        normalized.chop(1);
    }
    return normalized;
}

namespace
{
bool isOfficialApiUrl(const QString& url)
{
    return url.isEmpty() ||
           url == normalizeApiUrl(BuildConfig.MODRINTH_PROD_URL);
}

int modrinthDefaultPort(const QUrl& url)
{
    return url.scheme().compare("https", Qt::CaseInsensitive) == 0 ? 443 : 80;
}

// Whether the URL is on the same scheme, host and port as the API root, and
// under its path. Files of a source served elsewhere, even on the same host,
// don't get its token.
bool isUnderModrinthApiRoot(const QUrl& url, const QString& root)
{
    QUrl api(root);
    if (url.scheme().compare(api.scheme(), Qt::CaseInsensitive) != 0 ||
        url.host().compare(api.host(), Qt::CaseInsensitive) != 0 ||
        url.port(modrinthDefaultPort(url)) !=
            api.port(modrinthDefaultPort(api)))
    {
        return false;
    }

    auto api_path = api.path();
    while (api_path.endsWith('/'))
    {
        api_path.chop(1);
    }
    return api_path.isEmpty() || url.path() == api_path ||
           url.path().startsWith(api_path + '/');
}

QString modrinthSourceIconPath(const Source& source)
{
    return FS::PathCombine(APPLICATION->metacache()->getBasePath("general"),
                           "modrinth_sources", source.key() + ".png");
}

// The cached icons already loaded, by source key. Mod lists ask for them
// every time they're drawn.
QHash<QString, QIcon>& loadedSourceIcons()
{
    static QHash<QString, QIcon> icons;
    return icons;
}

// Finds the icon a source's website declares and caches it as a PNG. Labrinth
// front ends often still serve Modrinth's own favicon.ico, so only icons the
// page links to are used.
class ModrinthSourceIconFetcher : public QObject
{
public:
    explicit ModrinthSourceIconFetcher(Source source)
        : QObject(APPLICATION), m_source(std::move(source))
    {
    }

    void start()
    {
        m_page_job = new NetJob("Modrinth source page", APPLICATION->network());
        m_page_job->addNetAction(Net::Download::makeByteArray(
            QUrl(m_source.websiteRoot() + "/"), &m_page));
        connect(m_page_job.get(), &NetJob::finished, this,
                &ModrinthSourceIconFetcher::onPageFinished);
        m_page_job->start();
    }

private:
    void onPageFinished()
    {
        auto url = declaredIconUrl();
        if (!m_page_job->wasSuccessful() || !url.isValid())
        {
            deleteLater();
            return;
        }

        m_download_path = modrinthSourceIconPath(m_source) + ".download";
        m_icon_job = new NetJob("Modrinth source icon", APPLICATION->network());
        m_icon_job->addNetAction(Net::Download::makeFile(url, m_download_path));
        connect(m_icon_job.get(), &NetJob::finished, this,
                &ModrinthSourceIconFetcher::onIconFinished);
        m_icon_job->start();
    }

    void onIconFinished()
    {
        if (m_icon_job->wasSuccessful())
        {
            QImageReader reader(m_download_path);
            reader.setDecideFormatFromContent(true);
            // SVGs would otherwise be drawn at whatever size they declare
            if (reader.canRead() && reader.format().startsWith("svg"))
            {
                reader.setScaledSize(QSize(64, 64));
            }

            auto image = reader.read();
            if (image.isNull())
            {
                qWarning() << "The icon of the Modrinth source"
                           << m_source.displayName() << "isn't an image";
            }
            else
            {
                if (image.width() > 64 || image.height() > 64)
                {
                    image = image.scaled(64, 64, Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
                }
                image.save(modrinthSourceIconPath(m_source), "PNG");
                loadedSourceIcons().remove(m_source.key());
            }
        }

        QFile::remove(m_download_path);
        deleteLater();
    }

    QUrl declaredIconUrl() const
    {
        static const QRegularExpression link_tag(
            R"(<link\b[^>]*>)", QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression rel_attribute(
            R"(\brel\s*=\s*["']([^"']*)["'])",
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression href_attribute(
            R"(\bhref\s*=\s*["']([^"']*)["'])",
            QRegularExpression::CaseInsensitiveOption);

        QUrl page_url(m_source.websiteRoot() + "/");
        QUrl best;
        int best_rank = 0;

        auto tags = link_tag.globalMatch(QString::fromUtf8(m_page));
        while (tags.hasNext())
        {
            auto tag = tags.next().captured(0);
            auto rels = rel_attribute.match(tag).captured(1).toLower().split(
                ' ', Qt::SkipEmptyParts);
            auto href = href_attribute.match(tag).captured(1);
            if (href.isEmpty())
            {
                continue;
            }

            // Plain icons are meant for small sizes like ours, apple-touch-icon
            // is a large one for phones
            int rank = 0;
            if (rels.contains("icon"))
            {
                rank = 2;
            }
            else if (rels.contains("apple-touch-icon"))
            {
                rank = 1;
            }

            if (rank > best_rank)
            {
                best_rank = rank;
                best = page_url.resolved(QUrl(href));
            }
        }

        return best;
    }

    Source m_source;
    NetJob::Ptr m_page_job;
    NetJob::Ptr m_icon_job;
    QByteArray m_page;
    QString m_download_path;
};
}  // namespace

static QList<Source> parseCustomSources(const QString& json)
{
    QList<Source> sources;

    auto doc = QJsonDocument::fromJson(json.toUtf8());
    const auto array = doc.array();
    for (const auto& value : array)
    {
        auto obj = value.toObject();

        Source source;
        source.name = obj.value("name").toString().trimmed();
        source.api_url = normalizeApiUrl(obj.value("api_url").toString());
        source.website_url =
            normalizeApiUrl(obj.value("website_url").toString());
        source.token = obj.value("token").toString().trimmed();

        // Official Modrinth is always there, don't list it twice
        if (isOfficialApiUrl(source.api_url))
        {
            continue;
        }
        sources.append(source);
    }

    return sources;
}

QList<Source> customSources()
{
    // Every request asks for the token to send, so only parse the setting
    // again when it changes
    static QMutex mutex;
    static QString cached_json;
    static QList<Source> cached_sources;

    auto json = APPLICATION->settings()->get("ModrinthSources").toString();

    QMutexLocker locker(&mutex);
    if (cached_json.isNull() || json != cached_json)
    {
        cached_sources = parseCustomSources(json);
        cached_json = json;
    }
    return cached_sources;
}

void setCustomSources(const QList<Source>& sources)
{
    QJsonArray array;
    for (const auto& source : sources)
    {
        QJsonObject obj;
        obj.insert("name", source.name);
        obj.insert("api_url", normalizeApiUrl(source.api_url));
        obj.insert("website_url", normalizeApiUrl(source.website_url));
        if (!source.token.isEmpty())
        {
            obj.insert("token", source.token);
        }
        array.append(obj);
    }

    APPLICATION->settings()->set(
        "ModrinthSources",
        QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact)));
}

Source officialSource()
{
    Source source;
    source.token =
        APPLICATION->settings()->get("ModrinthToken").toString().trimmed();
    return source;
}

QList<Source> allSources()
{
    QList<Source> sources{officialSource()};
    sources.append(customSources());
    return sources;
}

std::optional<Source> findSource(const QString& url)
{
    auto normalized = normalizeApiUrl(url);
    if (isOfficialApiUrl(normalized))
    {
        return officialSource();
    }

    for (const auto& source : customSources())
    {
        if (source.api_url == normalized)
        {
            return source;
        }
    }

    return {};
}

QString tokenFor(const QUrl& url)
{
    for (const auto& source : allSources())
    {
        if (!source.token.isEmpty() &&
            isUnderModrinthApiRoot(url, source.isOfficial()
                                            ? BuildConfig.MODRINTH_PROD_URL
                                            : source.api_url))
        {
            return source.token;
        }
    }
    return {};
}

QIcon sourceIcon(const Source& source)
{
    if (source.isOfficial())
    {
        return APPLICATION->getThemedIcon("modrinth");
    }

    auto& loaded = loadedSourceIcons();
    auto cached = loaded.constFind(source.key());
    if (cached != loaded.constEnd())
    {
        return *cached;
    }

    auto path = modrinthSourceIconPath(source);
    if (QFile::exists(path))
    {
        QIcon icon(path);
        if (!icon.isNull())
        {
            loaded.insert(source.key(), icon);
            return icon;
        }
    }

    // Not Modrinth's icon, so the source can't be mistaken for it
    return APPLICATION->getThemedIcon("worlds");
}

void fetchSourceIcon(const Source& source, bool refresh)
{
    if (source.isOfficial())
    {
        return;
    }

    // Pages pick the icon up the next time they're made, so trying once per
    // session is enough
    static QSet<QString> tried;
    if (!refresh && (tried.contains(source.key()) ||
                     QFile::exists(modrinthSourceIconPath(source))))
    {
        return;
    }
    tried.insert(source.key());

    auto* fetcher = new ModrinthSourceIconFetcher(source);
    fetcher->start();
}

}  // namespace Modrinth
