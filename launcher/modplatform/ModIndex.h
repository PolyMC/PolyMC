// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
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

#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

class QIODevice;

namespace ModPlatform
{

enum class Provider
{
    MODRINTH,
    FLAME
};

class ProviderCapabilities
{
public:
    const char* name(Provider);
    QString readableName(Provider);
    QStringList hashType(Provider);
    QString hash(Provider, QIODevice*, QString type = "");
};

struct ModpackAuthor
{
    QString name;
    QString url;
};

struct DonationData
{
    QString id;
    QString platform;
    QString url;
};

enum class DependencyType
{
    Required,
    Optional,
    Incompatible,
    Embedded,
    Unknown
};

struct Dependency
{
    DependencyType type = DependencyType::Unknown;
    // A project on the same platform, and Modrinth source, as the version
    QVariant addonId;
    // A specific version of it, only on Modrinth
    QString version;

    // Some Modrinth-compatible sources, like 67minecraft, also name projects
    // on other platforms: external_source is "modrinth" or "curseforge", and
    // external_id the project ID there
    QString external_source;
    QString external_id;
    QString external_version;

    // A name and a link to show when it can't be installed
    QString name;
    QString url;
};

struct IndexedVersion
{
    QVariant addonId;
    QVariant fileId;
    QString version;
    QString version_number = {};
    QStringList mcVersion;
    QString downloadUrl;
    QString date;
    QString fileName;
    QStringList loaders = {};
    QString hash_type;
    QString hash;
    bool is_preferred = true;
    QString changelog;
    QList<Dependency> dependencies;
};

struct ExtraPackData
{
    QList<DonationData> donate;

    QString issuesUrl;
    QString sourceUrl;
    QString wikiUrl;
    QString discordUrl;

    QString body;
};

struct IndexedPack
{
    QVariant addonId;
    Provider provider;
    // API URL of the Modrinth-compatible source, empty for official Modrinth
    QString apiUrl;
    QString name;
    QString slug;
    QString description;
    QList<ModpackAuthor> authors;
    QString logoName;
    QString logoUrl;
    QString websiteUrl;

    bool versionsLoaded = false;
    QVector<IndexedVersion> versions;

    // Don't load by default, since some modplatform don't have that info
    bool extraDataLoaded = true;
    ExtraPackData extraData;
};

}  // namespace ModPlatform

Q_DECLARE_METATYPE(ModPlatform::IndexedPack)
Q_DECLARE_METATYPE(ModPlatform::Provider)
