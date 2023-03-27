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

#include "ATLShareCode.h"

namespace ATLauncher {

static void loadShareCodeMod(ShareCodeMod& m, nlohmann::json& obj)
{
    m.selected = obj["selected"];
    m.name = obj["name"].get<std::string>().c_str();
}

static void loadShareCode(ShareCode& c, nlohmann::json& obj)
{
    c.pack = obj["pack"].get<std::string>().c_str();
    c.version = obj["version"].get<std::string>().c_str();

    auto mods = obj["mods"];
    auto optional = mods["optional"];
    for (const auto& modRaw : optional) {
        auto modObj = modRaw;
        ShareCodeMod mod;
        loadShareCodeMod(mod, modObj);
        c.mods.append(mod);
    }
}

void loadShareCodeResponse(ShareCodeResponse& r, nlohmann::json& obj)
{
    r.error = obj["error"];
    r.code = obj["code"];

    if (obj.contains("message") && !obj["message"].is_null())
        r.message = obj["message"].get<std::string>().c_str();

    if (!r.error) {
        auto dataRaw = obj["data"];
        loadShareCode(r.data, dataRaw);
    }
}

}