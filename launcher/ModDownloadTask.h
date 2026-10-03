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
*/

#pragma once

#include "minecraft/mod/ResourceFolderModel.h"
#include "net/NetJob.h"
#include "tasks/SequentialTask.h"

#include "modplatform/ModIndex.h"
#include "minecraft/mod/tasks/LocalModUpdateTask.h"

class ModFolderModel;

class ModDownloadTask : public SequentialTask {
    Q_OBJECT
public:
    explicit ModDownloadTask(const ModPlatform::IndexedPack& mod,
                             const ModPlatform::IndexedVersion& version,
                             const std::shared_ptr<ResourceFolderModel> mods,
                             bool is_indexed = true);
    const QString& getFilename() const { return m_mod_version.fileName; }
    const QList<ModPlatform::Dependency> getDependencies() const {
        return m_mod_version.dependencies;
    }

    const ModPlatform::Provider getProvider() const {
        return m_mod.provider;
    }

    const QVariant& getAddonId() const {
        return m_mod.addonId;
    }

private:
    ModPlatform::IndexedPack m_mod;
    ModPlatform::IndexedVersion m_mod_version;
    const std::shared_ptr<ResourceFolderModel> mods;

    NetJob::Ptr m_filesNetJob;
    LocalModUpdateTask::Ptr m_update_task;

    void downloadProgressChanged(qint64 current, qint64 total);

    void downloadFailed(QString reason);

    void downloadSucceeded();

    std::tuple<QString, QString> to_delete {"", ""};

private slots:
    void hasOldMod(QString name, QString filename);
};



