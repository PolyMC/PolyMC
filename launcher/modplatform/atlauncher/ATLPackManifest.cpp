// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
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
 *      Copyright 2020-2021 Jamie Mansfield <jmansfield@cadixdev.org>
 *      Copyright 2021 Petr Mrazek <peterix@gmail.com>
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

#include "ATLPackManifest.h"

static ATLauncher::DownloadType parseDownloadType(const QString& rawType) {
    if(rawType == QString("server")) {
        return ATLauncher::DownloadType::Server;
    }
    else if(rawType == QString("browser")) {
        return ATLauncher::DownloadType::Browser;
    }
    else if(rawType == QString("direct")) {
        return ATLauncher::DownloadType::Direct;
    }

    return ATLauncher::DownloadType::Unknown;
}

static ATLauncher::ModType parseModType(const QString& rawType) {
    // See https://wiki.atlauncher.com/mod_types
    if(rawType == QString("root")) {
        return ATLauncher::ModType::Root;
    }
    else if(rawType == QString("forge")) {
        return ATLauncher::ModType::Forge;
    }
    else if(rawType == QString("jar")) {
        return ATLauncher::ModType::Jar;
    }
    else if(rawType == QString("mods")) {
        return ATLauncher::ModType::Mods;
    }
    else if(rawType == QString("flan")) {
        return ATLauncher::ModType::Flan;
    }
    else if(rawType == QString("dependency") || rawType == QString("depandency")) {
        return ATLauncher::ModType::Dependency;
    }
    else if(rawType == QString("ic2lib")) {
        return ATLauncher::ModType::Ic2Lib;
    }
    else if(rawType == QString("denlib")) {
        return ATLauncher::ModType::DenLib;
    }
    else if(rawType == QString("coremods")) {
        return ATLauncher::ModType::Coremods;
    }
    else if(rawType == QString("mcpc")) {
        return ATLauncher::ModType::MCPC;
    }
    else if(rawType == QString("plugins")) {
        return ATLauncher::ModType::Plugins;
    }
    else if(rawType == QString("extract")) {
        return ATLauncher::ModType::Extract;
    }
    else if(rawType == QString("decomp")) {
        return ATLauncher::ModType::Decomp;
    }
    else if(rawType == QString("texturepack")) {
        return ATLauncher::ModType::TexturePack;
    }
    else if(rawType == QString("resourcepack")) {
        return ATLauncher::ModType::ResourcePack;
    }
    else if(rawType == QString("shaderpack")) {
        return ATLauncher::ModType::ShaderPack;
    }
    else if(rawType == QString("texturepackextract")) {
        return ATLauncher::ModType::TexturePackExtract;
    }
    else if(rawType == QString("resourcepackextract")) {
        return ATLauncher::ModType::ResourcePackExtract;
    }
    else if(rawType == QString("millenaire")) {
        return ATLauncher::ModType::Millenaire;
    }

    return ATLauncher::ModType::Unknown;
}

static void loadVersionLibrary(ATLauncher::VersionLibrary& p, const nlohmann::json& obj)
{
    p.url = QString::fromStdString(obj["url"]);
    p.file = QString::fromStdString(obj["file"]);
    p.md5 = QString::fromStdString(obj["md5"]);

    p.download_raw = QString::fromStdString(obj["download"]);
    p.download = parseDownloadType(p.download_raw);

    p.server = QString::fromStdString(obj.value("server", ""));
}

static void loadVersionMod(ATLauncher::VersionMod& p, const nlohmann::json& obj)
{
    p.name = QString::fromStdString(obj["name"]);
    p.version = QString::fromStdString(obj["version"]);
    p.url = QString::fromStdString(obj["url"]);
    p.file = QString::fromStdString(obj["file"]);
    p.md5 = QString::fromStdString(obj.value("md5", ""));

    p.download_raw = QString::fromStdString(obj["download"]);
    p.download = parseDownloadType(p.download_raw);

    p.type_raw = QString::fromStdString(obj["type"]);
    p.type = parseModType(p.type_raw);

    // This contributes to the Minecraft Forge detection, where we rely on mod type being "Forge"
    // when the mod represents Forge. As there is little difference between "Jar" and "Forge", some
    // packs regretfully use "Jar". This will correct the type to "Forge" in these cases (as best
    // it can).
    if(p.name == QString("Minecraft Forge") && p.type == ATLauncher::ModType::Jar)
    {
        p.type_raw = "forge";
        p.type = ATLauncher::ModType::Forge;
    }

    if(obj.contains("extractTo"))
    {
        p.extractTo_raw = QString::fromStdString(obj["extractTo"]);
        p.extractTo = parseModType(p.extractTo_raw);
        p.extractFolder = QString::fromStdString(obj.value("extractFolder", "")).replace("%s%", "/");
    }

    if(obj.contains("decompType"))
    {
        p.decompType_raw = QString::fromStdString(obj["decompType"]);
        p.decompType = parseModType(p.decompType_raw);
        p.decompFile = QString::fromStdString(obj["decompFile"]);
    }

    p.description = QString::fromStdString(obj.value("description", ""));
    p.optional = obj.value("optional", false);
    p.recommended = obj.value("recommended", false);
    p.selected = obj.value("selected", false);
    p.hidden = obj.value("hidden", false);
    p.library = obj.value("library", false);
    p.group = QString::fromStdString(obj.value("group", ""));


    if(obj.contains("depends"))
    {
        auto dependsArr = obj["depends"];
        for (const auto& depends : dependsArr)
        {
            p.depends.append(QString::fromStdString(depends));
        }
    }
    p.colour = QString::fromStdString(obj.value("colour", ""));
    p.warning = QString::fromStdString(obj.value("warning", ""));

    p.client = obj.value("client", false);

    // computed
    p.effectively_hidden = p.hidden || p.library;
}

static void loadVersionKeeps(ATLauncher::VersionKeeps& k, nlohmann::json& obj)
{
    if (obj.contains("files"))
    {
        auto files = obj["files"];
        for (const auto& keepRaw : files)
        {
            ATLauncher::VersionKeep keep;
            keep.base = QString::fromStdString(keepRaw["base"]);
            keep.target = QString::fromStdString(keepRaw["target"]);
            k.files.append(keep);
        }
    }

    if (obj.contains("folders"))
    {
        auto folders = obj["folders"];
        for (const auto& keepRaw : folders)
        {
            ATLauncher::VersionKeep keep;
            keep.base = QString::fromStdString(keepRaw["base"]);
            keep.target = QString::fromStdString(keepRaw["target"]);
            k.folders.append(keep);
        }
    }
}

static void loadVersionDeletes(ATLauncher::VersionDeletes& d, nlohmann::json& obj)
{
    if (obj.contains("files"))
    {
        auto files = obj["files"];
        for (const auto& deleteRaw : files)
        {
            ATLauncher::VersionDelete versionDelete;
            versionDelete.base = QString::fromStdString(deleteRaw["base"]);
            versionDelete.target = QString::fromStdString(deleteRaw["target"]);
            d.files.append(versionDelete);
        }
    }

    if (obj.contains("folders"))
    {
        auto folders = obj["folders"];
        for (const auto& deleteRaw : folders)
        {
            ATLauncher::VersionDelete versionDelete;
            versionDelete.base = QString::fromStdString(deleteRaw["base"]);
            versionDelete.target = QString::fromStdString(deleteRaw["target"]);
            d.folders.append(versionDelete);
        }
    }
}

void ATLauncher::loadVersion(PackVersion & v, nlohmann::json& obj)
{
    v.version = QString::fromStdString(obj["version"]);
    v.minecraft = QString::fromStdString(obj["minecraft"]);
    v.noConfigs = obj.value("noConfigs", false);


    if(obj.contains("mainClass"))
    {
        v.mainClass.mainClass = QString::fromStdString(obj["mainClass"].value("mainClass", ""));
        v.mainClass.depends = QString::fromStdString(obj["mainClass"].value("depends", ""));
    }

    if(obj.contains("extraArguments"))
    {
        v.extraArguments.arguments = QString::fromStdString(obj["extraArguments"].value("arguments", ""));
        v.extraArguments.depends = QString::fromStdString(obj["extraArguments"].value("depends", ""));
    }

    if(obj.contains("loader"))
    {
        v.loader.type = QString::fromStdString(obj["loader"].value("type", ""));
        v.loader.choose = obj["loader"].value("choose", false);

        v.loader.latest = obj["loader"]["metadata"].value("latest", false);
        v.loader.recommended = obj["loader"]["metadata"].value("recommended", false);

        if (v.loader.type == "forge")
            v.loader.version = QString::fromStdString(obj["loader"]["metadata"].value("version", ""));

        if (v.loader.type == "fabric")
            v.loader.version = QString::fromStdString(obj["loader"]["metadata"].value("loader", ""));
    }

    if(obj.contains("libraries"))
    {
        for (const auto &libraryRaw : obj["libraries"])
        {
            ATLauncher::VersionLibrary target;
            loadVersionLibrary(target, libraryRaw);
            v.libraries.append(target);
        }

    }

    if(obj.contains("mods"))
    {
        for (const auto &modRaw : obj["mods"])
        {
            ATLauncher::VersionMod mod;
            loadVersionMod(mod, modRaw);
            v.mods.append(mod);
        }
    }

    if(obj.contains("configs"))
    {
        v.configs.filesize = obj["configs"].value("filesize", 0);
        v.configs.sha1 = QString::fromStdString(obj["configs"].value("sha1", ""));
    }

    auto colourObj = obj.value("colours", nlohmann::json::object());
    for (const auto &key : colourObj.items())
    {
        v.colours[QString::fromStdString(key.key())] = QString::fromStdString(key.value());
    }

    auto warningsObj = obj.value("warnings", nlohmann::json::object());
    for (const auto &key : warningsObj.items())
    {
        v.warnings[QString::fromStdString(key.key())] = QString::fromStdString(key.value());
    }

    if (obj.contains("messages"))
    {
        v.messages.install = QString::fromStdString(obj["messages"].value("install", ""));
        v.messages.update = QString::fromStdString(obj["messages"].value("update", ""));
    }

    if (obj.contains("keeps"))
    {
        loadVersionKeeps(v.keeps, obj["keeps"]);
    }

    if (obj.contains("deletes"))
    {
        loadVersionDeletes(v.deletes, obj["deletes"]);
    }
}
