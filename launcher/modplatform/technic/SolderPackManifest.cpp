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
 */

#include "SolderPackManifest.h"

namespace TechnicSolder {

void loadPack(Pack& v, nlohmann::json& obj)
{
    v.recommended = obj["recommended"].get<std::string>().c_str();
    v.latest = obj["latest"].get<std::string>().c_str();

    for (const auto& buildv : obj["builds"]) {
        v.builds.append(buildv.get<std::string>().c_str());
    }
}

static void loadPackBuildMod(PackBuildMod& b, const nlohmann::json& obj)
{
    b.name = obj["name"].get<std::string>().c_str();
    b.version = obj.value("version", "").c_str();
    b.md5 = obj["md5"].get<std::string>().c_str();
    b.url = obj["url"].get<std::string>().c_str();
}

void loadPackBuild(PackBuild& v, nlohmann::json& obj)
{
    v.minecraft = obj["minecraft"].get<std::string>().c_str();

    for (const auto& modObj : obj["mods"]) {
        PackBuildMod mod;
        loadPackBuildMod(mod, modObj);
        v.mods.append(mod);
    }
}

}
