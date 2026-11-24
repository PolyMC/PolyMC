// SPDX-License-Identifier: GPL-3.0-only

#include "ModrinthSource.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include "Application.h"
#include "BuildConfig.h"
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
}  // namespace

QList<Source> customSources()
{
    QList<Source> sources;

    auto doc = QJsonDocument::fromJson(
        APPLICATION->settings()->get("ModrinthSources").toString().toUtf8());
    const auto array = doc.array();
    for (const auto& value : array)
    {
        auto obj = value.toObject();

        Source source;
        source.name = obj.value("name").toString().trimmed();
        source.api_url = normalizeApiUrl(obj.value("api_url").toString());
        source.website_url =
            normalizeApiUrl(obj.value("website_url").toString());

        // Official Modrinth is always there, don't list it twice
        if (isOfficialApiUrl(source.api_url))
        {
            continue;
        }
        sources.append(source);
    }

    return sources;
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
        array.append(obj);
    }

    APPLICATION->settings()->set(
        "ModrinthSources",
        QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact)));
}

QList<Source> allSources()
{
    QList<Source> sources{Source{}};
    sources.append(customSources());
    return sources;
}

std::optional<Source> findSource(const QString& url)
{
    auto normalized = normalizeApiUrl(url);
    if (isOfficialApiUrl(normalized))
    {
        return Source{};
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

}  // namespace Modrinth
