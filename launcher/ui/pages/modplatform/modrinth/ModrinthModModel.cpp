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
 */

#include "ModrinthModModel.h"

#include <QDebug>

#include "Application.h"
#include "Json.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "modplatform/modrinth/ModrinthAPI.h"
#include "modplatform/modrinth/ModrinthExtraProjects.h"
#include "modplatform/modrinth/ModrinthPackIndex.h"

namespace Modrinth
{

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
const char* ListModel::sorts[5]{"relevance", "downloads", "follows", "updated",
                                "newest"};

ListModel::ListModel(ModrinthModPage* parent)
    : ModPlatform::ListModel(parent), m_extra(new ModrinthExtraProjects(this))
{
    connect(m_extra, &ModrinthExtraProjects::projectsFound, this,
            &ListModel::onExtraProjects);
}

void ListModel::searchExtra()
{
    auto* api = static_cast<const ModrinthAPI*>(m_parent->apiProvider());

    ModrinthExtraProjects::Filter filter;
    filter.project_type = ModrinthAPI::getProjectType(m_parent->resourceType());
    if (auto* instance = dynamic_cast<MinecraftInstance*>(m_parent->m_instance))
    {
        auto profile = instance->getPackProfile();
        filter.minecraft_version =
            profile->getComponentVersion("net.minecraft");
        if (m_parent->resourceType() == ModAPI::Mod)
        {
            filter.loaders =
                ModrinthAPI::getModLoaderStrings(profile->getModLoaders());
        }
    }

    m_extra->search(*api, currentSearchTerm, filter);
}

void ListModel::onExtraProjects(QList<QJsonObject> projects)
{
    QList<ModPlatform::IndexedPack> packs;
    for (auto& project : projects)
    {
        ModPlatform::IndexedPack pack;
        try
        {
            loadIndexedPack(pack, project);
        }
        catch (const JSONValidationError& e)
        {
            qWarning() << "Error while loading a project from"
                       << m_parent->debugName() << ":" << e.cause();
            continue;
        }

        auto status = ModrinthExtraProjects::statusLabel(project);
        if (!status.isEmpty())
        {
            pack.description = QString("[%1] %2").arg(status, pack.description);
        }
        packs.append(pack);
    }

    addExtraPacks(packs);
}

void ListModel::loadIndexedPack(ModPlatform::IndexedPack& m, QJsonObject& obj)
{
    // Modrinth list models only live on Modrinth pages
    auto* api = static_cast<const ModrinthAPI*>(m_parent->apiProvider());
    Modrinth::loadIndexedPack(m, obj, ModAPI::Mod, api);
}

void ListModel::loadExtraPackInfo(ModPlatform::IndexedPack& m, QJsonObject& obj)
{
    Modrinth::loadExtraPackData(m, obj);
}

void ListModel::loadIndexedPackVersions(ModPlatform::IndexedPack& m,
                                        QJsonArray& arr)
{
    Modrinth::loadIndexedPackVersions(m, arr, APPLICATION->network(),
                                      m_parent->m_instance);
}

QJsonArray ListModel::documentToArray(QJsonDocument& obj) const
{
    return obj.object().value("hits").toArray();
}

}  // namespace Modrinth

#include "moc_ModrinthModModel.cpp"
