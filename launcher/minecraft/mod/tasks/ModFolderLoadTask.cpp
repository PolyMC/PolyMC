// SPDX-License-Identifier: GPL-3.0-only
/*
*  PolyMC - Minecraft Launcher
*  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
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
*      Copyright 2013-2021 MultiMC Contributors
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

#include "ModFolderLoadTask.h"

#include "minecraft/mod/MetadataHandler.h"

ModFolderLoadTask::ModFolderLoadTask(QDir& mods_dir, QDir& index_dir, bool is_indexed) 
    : m_mods_dir(mods_dir), m_index_dir(index_dir), m_is_indexed(is_indexed), m_result(new Result())
{}

void ModFolderLoadTask::run()
{
    if (m_is_indexed) {
        // Read metadata first
        getFromMetadata();
    }

    // Read JAR files that don't have metadata
    m_mods_dir.refresh();
    for (auto entry : m_mods_dir.entryInfoList()) {
        Mod::Ptr mod(new Mod(entry));

        if (mod->enabled()) {
            if (m_result->mods.contains(mod->internal_id())) {
                m_result->mods[mod->internal_id()]->setStatus(ModStatus::Installed);
            }
            else {
                m_result->mods[mod->internal_id()] = mod;
                m_result->mods[mod->internal_id()]->setStatus(ModStatus::NoMetadata);
            }
        }
        else { 
            QString chopped_id = mod->internal_id().chopped(9);
            if (m_result->mods.contains(chopped_id)) {
                m_result->mods[mod->internal_id()] = mod;

                auto metadata = m_result->mods[chopped_id]->metadata();
                if (metadata) {
                    mod->setMetadata(*metadata);

                    m_result->mods[mod->internal_id()]->setStatus(ModStatus::Installed);
                    m_result->mods.remove(chopped_id);
                }
            }
            else {
                m_result->mods[mod->internal_id()] = mod;
                m_result->mods[mod->internal_id()]->setStatus(ModStatus::NoMetadata);
            }
        }
    }

    emit succeeded();
}

void ModFolderLoadTask::getFromMetadata()
{
    m_index_dir.refresh();
    for (auto entry : m_index_dir.entryList(QDir::Files)) {
        auto metadata = Metadata::get(m_index_dir, entry);

        if(!metadata.isValid()){
            return;
        }

        auto* mod = new Mod(m_mods_dir, metadata);
        mod->setStatus(ModStatus::NotInstalled);
        m_result->mods[mod->internal_id()] = mod;
    }
}
