// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2026 crueter <crueter@crueter.xyz>
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

#pragma once

#include "Application.h"
#include "DownloadableResourcesPage.h"

#include "minecraft/mod/DataPackFolderModel.h"
#include "ui_ExternalResourcesPage.h"

class DataPackPage : public DownloadableResourcesPage {
    Q_OBJECT
public:
    explicit DataPackPage(MinecraftInstance* instance,
                              std::shared_ptr<DataPackFolderModel> model, QWidget* parent = 0);
    virtual ~DataPackPage() {}

    QString displayName() const override {
        return tr("Data packs");
    }
    QIcon icon() const override {
        return APPLICATION->getThemedIcon("datapacks");
    }
    QString id() const override {
        return "datapacks";
    }
    QString helpPage() const override {
        return "Data-packs";
    }

    bool shouldDisplay() const override {
        // FIXME(crueter) newer vers only?
        return true;
    }

public slots:
    bool onSelectionChanged(const QModelIndex& current, const QModelIndex& previous) override {
        auto sourceCurrent = m_filterModel->mapToSource(current);
        int row = sourceCurrent.row();
        auto& rp = static_cast<DataPack&>(m_model->at(row));
        ui->frame->updateWithResourcePack(rp);

        return true;
    }

private slots:
    void installDataPacks();
};
