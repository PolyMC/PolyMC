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

#include "ModrinthPackIndex.h"
#include "ModrinthAPI.h"

#include "Json.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "net/NetJob.h"

static ModrinthAPI api;
static ModPlatform::ProviderCapabilities ProviderCaps;

void Modrinth::loadIndexedPack(ModPlatform::IndexedPack& pack, nlohmann::json& obj)
{
    pack.addonId = obj.value("project_id", "").c_str();
    if (pack.addonId.toString().isEmpty())
        pack.addonId = obj.value("id", "").c_str();

    pack.provider = ModPlatform::Provider::MODRINTH;
    pack.name = obj.value("title", "").c_str();

    pack.slug = obj.value("slug", "").c_str();
    if (!pack.slug.isEmpty())
        pack.websiteUrl = "https://modrinth.com/mod/" + pack.slug;
    else
        pack.websiteUrl = "";

    pack.description = obj.value("description", "").c_str();

    nlohmann::json temp;

    temp = obj.value("icon_url", nlohmann::json());
    if (!temp.is_null()) {
        pack.logoUrl = temp.get<std::string>().c_str();
    }

    pack.logoName = pack.addonId.toString();

    ModPlatform::ModpackAuthor modAuthor;
    temp = obj.value("authors", nlohmann::json());
    if (!temp.is_null()) {
        modAuthor.name = temp.get<std::string>().c_str();
    }

    modAuthor.url = api.getAuthorURL(modAuthor.name);
    pack.authors.append(modAuthor);

    // Modrinth can have more data than what's provided by the basic search :)
    pack.extraDataLoaded = false;
}

void Modrinth::loadExtraPackData(ModPlatform::IndexedPack& pack, nlohmann::json& obj)
{
    nlohmann::json temp;

    temp = obj.value("issues_url", nlohmann::json());
    if (!temp.is_null()) {
        pack.extraData.issuesUrl = temp.get<std::string>().c_str();
    } else {
        pack.extraData.issuesUrl = "";
    }
    if (pack.extraData.issuesUrl.endsWith('/'))
        pack.extraData.issuesUrl.chop(1);

    temp = obj.value("source_url", nlohmann::json());
    if (!temp.is_null()) {
        pack.extraData.sourceUrl = temp.get<std::string>().c_str();
    } else {
        pack.extraData.sourceUrl = "";
    }
    if (pack.extraData.sourceUrl.endsWith('/'))
        pack.extraData.sourceUrl.chop(1);

    temp = obj.value("wiki_url", nlohmann::json());
    if (!temp.is_null()) {
        pack.extraData.wikiUrl = temp.get<std::string>().c_str();
    } else {
        pack.extraData.wikiUrl = "";
    }
    if (pack.extraData.wikiUrl.endsWith('/'))
        pack.extraData.wikiUrl.chop(1);

    temp = obj.value("discord_url", nlohmann::json());
    if (!temp.is_null()) {
        pack.extraData.discordUrl = temp.get<std::string>().c_str();
    } else {
        pack.extraData.discordUrl = "";
    }
    if (pack.extraData.discordUrl.endsWith('/'))
        pack.extraData.discordUrl.chop(1);

    temp = obj.value("donate_urls", nlohmann::json());
    if (!temp.is_null()) {
        auto donate_arr = temp;
        for (const auto& d : donate_arr) {
            ModPlatform::DonationData donate;
            donate.id = d.value("id", "").c_str();
            donate.platform = d.value("platform", "").c_str();
            donate.url = d.value("url", "").c_str();
            pack.extraData.donate.append(donate);
        }
    }

    temp = obj.value("body", nlohmann::json());
    if (!temp.is_null()) {
        pack.extraData.body = temp.get<std::string>().c_str();
    } else {
        pack.extraData.body = "";
    }

    pack.extraDataLoaded = true;
}

void Modrinth::loadIndexedPackVersions(ModPlatform::IndexedPack& pack,
                                       QJsonArray& arr,
                                       const shared_qobject_ptr<QNetworkAccessManager>& network,
                                       BaseInstance* inst)
{
    QVector<ModPlatform::IndexedVersion> unsortedVersions;
    QString mcVersion = (static_cast<MinecraftInstance*>(inst))->getPackProfile()->getComponentVersion("net.minecraft");

    for (auto versionIter : arr) {
        auto obj = versionIter.toObject();
        auto file = loadIndexedPackVersion(obj);

        if(file.fileId.isValid()) // Heuristic to check if the returned value is valid
            unsortedVersions.append(file);
    }
    auto orderSortPredicate = [](const ModPlatform::IndexedVersion& a, const ModPlatform::IndexedVersion& b) -> bool {
        // dates are in RFC 3339 format
        return a.date > b.date;
    };
    std::sort(unsortedVersions.begin(), unsortedVersions.end(), orderSortPredicate);
    pack.versions = unsortedVersions;
    pack.versionsLoaded = true;
}

auto Modrinth::loadIndexedPackVersion(QJsonObject &obj, QString preferred_hash_type, QString preferred_file_name) -> ModPlatform::IndexedVersion
{
    ModPlatform::IndexedVersion file;

    file.addonId = Json::requireString(obj, "project_id");
    file.fileId = Json::requireString(obj, "id");
    file.date = Json::requireString(obj, "date_published");
    auto versionArray = Json::requireArray(obj, "game_versions");
    if (versionArray.empty()) {
        return {};
    }
    for (auto mcVer : versionArray) {
        file.mcVersion.append(mcVer.toString());
    }
    auto loaders = Json::requireArray(obj, "loaders");
    for (auto loader : loaders) {
        file.loaders.append(loader.toString());
    }
    file.version = Json::requireString(obj, "name");
    file.version_number = Json::requireString(obj, "version_number");
    file.changelog = Json::requireString(obj, "changelog");

    auto files = Json::requireArray(obj, "files");
    int i = 0;

    // Find correct file (needed in cases where one version may have multiple files)
    // Will default to the last one if there's no primary (though I think Modrinth requires that
    // at least one file is primary, idk)
    // NOTE: files.count() is 1-indexed, so we need to subtract 1 to become 0-indexed
    while (i < files.count() - 1) {
        auto parent = files[i].toObject();
        auto fileName = Json::requireString(parent, "filename");

        if (!preferred_file_name.isEmpty() && fileName.contains(preferred_file_name)) {
            file.is_preferred = true;
            break;
        }

        // Grab the primary file, if available
        if (Json::requireBoolean(parent, "primary"))
            break;

        i++;
    }

    auto parent = files[i].toObject();
    if (parent.contains("url")) {
        file.downloadUrl = Json::requireString(parent, "url");
        file.fileName = Json::requireString(parent, "filename");
        file.is_preferred = Json::requireBoolean(parent, "primary") || (files.count() == 1);
        auto hash_list = Json::requireObject(parent, "hashes");
        
        if (hash_list.contains(preferred_hash_type)) {
            file.hash = Json::requireString(hash_list, preferred_hash_type);
            file.hash_type = preferred_hash_type;
        } else {
            auto hash_types = ProviderCaps.hashType(ModPlatform::Provider::MODRINTH);
            for (auto& hash_type : hash_types) {
                if (hash_list.contains(hash_type)) {
                    file.hash = Json::requireString(hash_list, hash_type);
                    file.hash_type = hash_type;
                    break;
                }
            }
        }

        return file;
    }

    return {};
}
