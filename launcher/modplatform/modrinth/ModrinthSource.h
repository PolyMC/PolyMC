// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QString>

namespace Modrinth
{

// A site serving the Modrinth API: official Modrinth, or a Modrinth-compatible
// one such as a self-hosted Labrinth. Project and version IDs only mean
// something within the source they came from.
struct Source
{
    QString name;
    // Versioned API root, like https://api.modrinth.com/v2. Empty means
    // official Modrinth.
    QString api_url;
    // Empty means the scheme and host of the API URL
    QString website_url;

    bool isOfficial() const
    {
        return api_url.isEmpty();
    }

    QString displayName() const;
    QString websiteRoot() const;

    // Short and stable, used in page IDs and cache entry names
    QString key() const;
    // Keeps cached files of projects with the same ID on different sources
    // apart
    QString cacheName(const QString& id) const;
};

// Trims whitespace and trailing slashes so equal URLs compare equal
QString normalizeApiUrl(const QString& url);

}  // namespace Modrinth
