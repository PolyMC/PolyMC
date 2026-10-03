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

#include "BuildConfig.h"
#include "modplatform/ModAPI.h"
#include "modplatform/helpers/NetworkModAPI.h"

#include <QDebug>

class ModrinthAPI : public NetworkModAPI {
   public:
    auto currentVersion(QString hash,
                        QString hash_format,
                        QByteArray* response) -> NetJob::Ptr;

    auto currentVersions(const QStringList& hashes,
                         QString hash_format,
                         QByteArray* response) -> NetJob::Ptr;

    auto latestVersion(QString hash,
                       QString hash_format,
                       std::list<Version> mcVersions,
                       ModLoaderTypes loaders,
                       QByteArray* response) -> NetJob::Ptr;

    auto latestVersions(const QStringList& hashes,
                        QString hash_format,
                        std::list<Version> mcVersions,
                        ModLoaderTypes loaders,
                        QByteArray* response) -> NetJob::Ptr;

    auto getProjects(QStringList addonIds, QByteArray* response) const -> NetJob* override;

   public:
    inline auto getAuthorURL(const QString& name) const -> QString { return "https://modrinth.com/user/" + name; };

    inline QString getProjectType(ResourceType type) const {
        switch (type) {
        case ResourcePack:
            return "resourcepack";
        case ShaderPack:
            return "shader";
        default:
            return "mod";
        }
    }

    static auto getModLoaderStrings(const ModLoaderTypes types) -> const QStringList
    {
        QStringList l;
        for (auto loader : {Forge, NeoForge, Fabric, Quilt})
        {
            if ((types & loader) || types == Unspecified)
            {
                l << ModAPI::getModLoaderString(loader);
            }
        }
        if ((types & Quilt) && (~types & Fabric))  // Add Fabric if Quilt is in use, if Fabric isn't already there
            l << ModAPI::getModLoaderString(Fabric);
        return l;
    }

    static auto getModLoaderFilters(ModLoaderTypes types) -> const QString
    {
        QStringList l;
        for (const auto &loader : getModLoaderStrings(types))
        {
            l << QString("\"categories:%1\"").arg(loader);
        }
        return l.join(',');
    }

   private:
    inline auto getModSearchURL(SearchArgs& args) const -> QString override
    {
        if (args.type == Mod && !validateModLoaders(args.loaders)) {
            qWarning() << "Modrinth only have Forge, NeoForge and Fabric-compatible mods!";
            return "";
        }

        const auto off = QString::number(args.offset);
        const auto filter = args.type == Mod ? QStringLiteral("[%1],").arg(getModLoaderFilters(args.loaders)) : "";
        const auto projType = getProjectType(args.type);

        return QString(BuildConfig.MODRINTH_PROD_URL + "/search?"
                                                       "offset=%1&"
                                                       "limit=25&"
                                                       "query=%2&"
                                                       "index=%3&"
                                                       "facets=[%4%5[\"project_type:%6\"]]")
            .arg(off, args.search, args.sorting, filter,
                 getGameVersionsArray(args.versions), projType);
    };

    inline auto getModInfoURL(QString& id) const -> QString override
    {
        return BuildConfig.MODRINTH_PROD_URL + "/project/" + id;
    };

    inline auto getMultipleModInfoURL(QStringList ids) const -> QString
    {
        return BuildConfig.MODRINTH_PROD_URL + QString("/projects?ids=[\"%1\"]").arg(ids.join("\",\""));
    };

    inline auto getVersionsURL(VersionSearchArgs& args) const -> QString override
    {
        const auto loaders = args.type == Mod
                                 ? QStringLiteral("&loaders=[\"%1\"]")
                                       .arg(getModLoaderStrings(args.loaders).join("\",\""))
                                 : "";
        return QString(BuildConfig.MODRINTH_PROD_URL + "/project/%1/version?"
                                                       "game_versions=[%2]"
                                                       "%3")
            .arg(args.addonId, getGameVersionsString(args.mcVersions), loaders);
    };

    auto getGameVersionsArray(std::list<Version> mcVersions) const -> QString
    {
        QString s;
        for(auto& ver : mcVersions){
            s += QString("\"versions:%1\",").arg(ver.toString());
        }
        s.remove(s.length() - 1, 1); //remove last comma
        return s.isEmpty() ? QString() : QString("[%1],").arg(s);
    }

    inline auto validateModLoaders(ModLoaderTypes loaders) const -> bool
    {
        return (loaders == Unspecified) || (loaders & (Forge | NeoForge | Fabric | Quilt));
    }

};
