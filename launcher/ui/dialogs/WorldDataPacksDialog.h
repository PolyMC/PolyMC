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
 */

#pragma once

#include <QDialog>
#include <QVBoxLayout>

#include "minecraft/mod/DataPackFolderModel.h"

class MinecraftInstance;
class DataPackPage;

class WorldDataPacksDialog : public QDialog {
    Q_OBJECT
public:
    WorldDataPacksDialog(MinecraftInstance* inst, const QString& worldName,
                         const QString& datapacksDir, QWidget* parent);

    ~WorldDataPacksDialog();

private:
    const std::shared_ptr<DataPackFolderModel> m_model;
    QVBoxLayout *m_verticalLayout;
    DataPackPage *m_page;
};
