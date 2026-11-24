// SPDX-License-Identifier: GPL-3.0-only

#include "ModrinthSource.h"

#include <QCryptographicHash>
#include <QUrl>

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

}  // namespace Modrinth
