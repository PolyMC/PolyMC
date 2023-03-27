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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *      Copyright 2022 kb1000
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "ModrinthPackManifest.h"

#include "modplatform/modrinth/ModrinthAPI.h"

#include "minecraft/MinecraftInstance.h"

#include <QSet>

static ModrinthAPI api;

namespace Modrinth {

void loadIndexedPack(Modpack& pack, const nlohmann::json& obj)
{
    pack.id = obj.value("project_id", "").c_str();
    pack.name = obj.value("title", "").c_str();
    pack.description = obj.value("description", "").c_str();
    auto temp_author_name = obj.value("author", "").c_str();
    pack.author = std::make_tuple(temp_author_name, api.getAuthorURL(temp_author_name));
    pack.iconName = QString("modrinth_%1").arg(obj.value("slug", "").c_str());
    pack.iconUrl = obj.value("icon_url", "").c_str();
}

void loadIndexedInfo(Modpack& pack, nlohmann::json& obj)
{
    pack.extra.body = obj["body"].get<std::string>().c_str();
    pack.extra.projectUrl = QString("https://modrinth.com/modpack/%1").arg(obj["slug"].get<std::string>().c_str());

    nlohmann::json temp;

    temp = obj["issues_url"];
    if(temp.is_string())
    {
        pack.extra.issuesUrl = temp.get<std::string>().c_str();
        if(pack.extra.issuesUrl.endsWith('/'))
            pack.extra.issuesUrl.chop(1);
    }

    temp = obj["source_url"];
    if(temp.is_string())
    {
        pack.extra.sourceUrl = temp.get<std::string>().c_str();
        if(pack.extra.sourceUrl.endsWith('/'))
            pack.extra.sourceUrl.chop(1);
    }

    temp = obj["wiki_url"];
    if(temp.is_string())
    {
        pack.extra.wikiUrl = temp.get<std::string>().c_str();
        if(pack.extra.wikiUrl.endsWith('/'))
            pack.extra.wikiUrl.chop(1);
    }

    temp = obj["discord_url"];
    if(temp.is_string())
    {
        pack.extra.discordUrl = temp.get<std::string>().c_str();
        if(pack.extra.discordUrl.endsWith('/'))
            pack.extra.discordUrl.chop(1);
    }

    auto donate_arr = obj["donate_urls"];
    for (const auto& d_obj : donate_arr){
        DonationData donate;

        donate.id = d_obj["id"].get<std::string>().c_str();
        donate.platform = d_obj["platform"].get<std::string>().c_str();
        donate.url = d_obj["url"].get<std::string>().c_str();

        pack.extra.donate.append(donate);
    }

    pack.extraInfoLoaded = true;
}

void loadIndexedVersions(Modpack& pack, nlohmann::json& doc)
{
    QVector<ModpackVersion> unsortedVersions;

    for (const auto& obj : doc) {
        auto file = loadIndexedVersion(obj);

        if(!file.id.isEmpty()) // Heuristic to check if the returned value is valid
            unsortedVersions.append(file);
    }
    auto orderSortPredicate = [](const ModpackVersion& a, const ModpackVersion& b) -> bool {
        // dates are in RFC 3339 format
        return a.date > b.date;
    };

    std::sort(unsortedVersions.begin(), unsortedVersions.end(), orderSortPredicate);

    pack.versions.swap(unsortedVersions);

    pack.versionsLoaded = true;
}

auto loadIndexedVersion(const nlohmann::json& obj) -> ModpackVersion
{
    ModpackVersion file;

    file.name = obj["name"].get<std::string>().c_str();
    file.version = obj["version_number"].get<std::string>().c_str();
    file.id = obj["id"].get<std::string>().c_str();
    file.project_id = obj["project_id"].get<std::string>().c_str();
    file.date = obj["date_published"].get<std::string>().c_str();

    for (const auto& parent : obj["files"]) {
        File indexed_file;
        auto is_primary = parent.value("primary", false);
        if (!is_primary) {
            QString filename = parent["filename"].get<std::string>().c_str();
            // Checking suffix here is fine because it's the response from Modrinth,
            // so one would assume it will always be in English.
            if(!filename.endsWith("mrpack") && !filename.endsWith("zip"))
                continue;
        }

        file.download_url = parent["url"].get<std::string>().c_str();
        if(is_primary)
            break;
    }

    if(file.download_url.isEmpty())
        return {};

    return file;
}        

}  // namespace Modrinth
