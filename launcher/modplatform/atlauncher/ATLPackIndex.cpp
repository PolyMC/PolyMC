/*
 * Copyright 2020-2021 Jamie Mansfield <jmansfield@cadixdev.org>
 * Copyright 2021 Petr Mrazek <peterix@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ATLPackIndex.h"

#include <QRegularExpression>

static void loadIndexedVersion(ATLauncher::IndexedVersion& v, nlohmann::json& obj)
{
    v.version = obj["version"].get<std::string>().c_str();
    v.minecraft = obj["minecraft"].get<std::string>().c_str();
}

void ATLauncher::loadIndexedPack(ATLauncher::IndexedPack& m, nlohmann::json& obj)
{
        m.id = obj["id"];
        m.position = obj["position"];
        m.name = obj.value("name", "").c_str();
        m.type = obj["type"] == "private" ? ATLauncher::PackType::Private : ATLauncher::PackType::Public;
        for (auto versionRaw : obj["versions"]) {
            ATLauncher::IndexedVersion version;
            loadIndexedVersion(version, versionRaw);
            m.versions.append(version);
        }
        m.system = obj.value("system", false);

        //.value() excepts, no clue why
        if (obj.contains("description") && obj["description"].is_string())
            m.description = obj["description"].get<std::string>().c_str();
        else
            m.description = "";

        QString name = obj.value("name", "").c_str();
        m.safeName = name.replace(QRegularExpression("[^A-Za-z0-9]"), "");
}