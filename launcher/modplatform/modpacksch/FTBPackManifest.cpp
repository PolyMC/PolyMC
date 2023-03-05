// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
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
 *      Copyright 2020 Jamie Mansfield <jmansfield@cadixdev.org>
 *      Copyright 2020-2021 Petr Mrazek <peterix@gmail.com>
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

#include "FTBPackManifest.h"


static void loadSpecs(ModpacksCH::Specs& s, const nlohmann::json& obj)
{
    s.id = obj["id"].get<int>();
    s.minimum = obj["minimum"].get<int>();
    s.recommended = obj["recommended"].get<int>();
}

static void loadTag(ModpacksCH::Tag& t, const nlohmann::json& obj)
{
    t.id = obj["id"].get<int>();
    t.name = obj["name"].get<std::string>().c_str();
}

static void loadArt(ModpacksCH::Art& a, const nlohmann::json& obj)
{
    a.id = obj["id"].get<int>();
    a.url = obj["url"].get<std::string>().c_str();
    a.type = obj["type"].get<std::string>().c_str();
    a.width = obj["width"].get<int>();
    a.height = obj["height"].get<int>();
    a.compressed = obj["compressed"].get<bool>();
    a.sha1 = obj["sha1"].get<std::string>().c_str();
    a.size = obj["size"].get<int>();
    a.updated = obj["updated"].get<int>();
}

static void loadAuthor(ModpacksCH::Author & a, const nlohmann::json& obj)
{
    a.id = obj["id"].get<int>();
    a.name = obj["name"].get<std::string>().c_str();
    a.type = obj["type"].get<std::string>().c_str();
    a.website = obj["website"].get<std::string>().c_str();
    a.updated = obj["updated"].get<int>();
}

static void loadVersionInfo(ModpacksCH::VersionInfo& v, const nlohmann::json& obj)
{
    v.id = obj["id"].get<int>();
    v.name = obj["name"].get<std::string>().c_str();
    v.type = obj["type"].get<std::string>().c_str();
    v.updated = obj["updated"].get<int>();
    loadSpecs(v.specs, obj["specs"]);
}

void ModpacksCH::loadModpack(ModpacksCH::Modpack& m, const nlohmann::json& obj)
{
        m.id = obj["id"].get<int>();
        m.name = obj["name"].get<std::string>().c_str();
        m.synopsis = obj["synopsis"].get<std::string>().c_str();
        m.description = obj["description"].get<std::string>().c_str();
        m.type = obj["type"].get<std::string>().c_str();
        m.featured = obj["featured"].get<bool>();
        m.installs = obj["installs"].get<int>();
        m.plays = obj["plays"].get<int>();
        m.updated = obj["updated"].get<int>();
        m.refreshed = obj["refreshed"].get<int>();
        for (const auto& artRaw : obj["art"])
        {
            ModpacksCH::Art art;
            loadArt(art, artRaw);
            m.art.append(art);
        }
        for (const auto& authorRaw : obj["authors"])
        {
            ModpacksCH::Author author;
            loadAuthor(author, authorRaw);
            m.authors.append(author);
        }
        for (const auto& versionRaw : obj["versions"])
        {
            ModpacksCH::VersionInfo version;
            loadVersionInfo(version, versionRaw);
            m.versions.append(version);
        }
        for (const auto& tagRaw : obj["tags"])
        {
            ModpacksCH::Tag tag;
            loadTag(tag, tagRaw);
            m.tags.append(tag);
        }
        m.updated = obj["updated"].get<int>();
}


static void loadVersionTarget(ModpacksCH::VersionTarget& a, const nlohmann::json& obj)
{
    a.id = obj["id"].get<int>();
    a.name = obj["name"].get<std::string>().c_str();
    a.type = obj["type"].get<std::string>().c_str();
    a.version = obj["version"].get<std::string>().c_str();
    a.updated = obj["updated"].get<int>();
}

static void loadVersionFile(ModpacksCH::VersionFile& a, const nlohmann::json& obj)
{
    a.id = obj["id"].get<int>();
    a.type = obj["type"].get<std::string>().c_str();
    a.path = obj["path"].get<std::string>().c_str();
    a.name = obj["name"].get<std::string>().c_str();
    a.version = obj["version"].get<std::string>().c_str();
    a.url = obj.value("url", "").c_str();
    a.sha1 = obj["sha1"].get<std::string>().c_str();
    a.size = obj["size"].get<int>();
    a.clientOnly = obj["clientonly"].get<bool>();
    a.serverOnly = obj["serveronly"].get<bool>();
    a.optional = obj["optional"].get<bool>();
    a.updated = obj["updated"].get<int>();
    auto curseforgeObj = obj.value("curseforge", nlohmann::json());
    if (curseforgeObj.contains("project") && curseforgeObj.contains("file"))
    {
        a.curseforge.project_id = curseforgeObj["project"].get<int>();
        a.curseforge.file_id = curseforgeObj["file"].get<int>();
    }
    else
    {
        a.curseforge.project_id = 0;
        a.curseforge.file_id = 0;
    }
}

void ModpacksCH::loadVersion(ModpacksCH::Version& m, const nlohmann::json& obj)
{
    m.id = obj["id"].get<int>();
    m.parent = obj["parent"].get<int>();
    m.name = obj["name"].get<std::string>().c_str();
    m.type = obj["type"].get<std::string>().c_str();
    m.installs = obj["installs"].get<int>();
    m.plays = obj["plays"].get<int>();
    m.updated = obj["updated"].get<int>();
    m.refreshed = obj["refreshed"].get<int>();
    loadSpecs(m.specs, obj["specs"]);
    for (auto& targetRaw : obj["targets"])
    {
        ModpacksCH::VersionTarget target;
        loadVersionTarget(target, targetRaw);
        m.targets.append(target);
    }
    for (auto& fileRaw : obj["files"])
    {
        ModpacksCH::VersionFile file;
        loadVersionFile(file, fileRaw);
        m.files.append(file);
    }
}

//static void loadVersionChangelog(ModpacksCH::VersionChangelog & m, QJsonObject & obj)
//{
//    m.content = Json::requireString(obj, "content");
//    m.updated = Json::requireInteger(obj, "updated");
//}
