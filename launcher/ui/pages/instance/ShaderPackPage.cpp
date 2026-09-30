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

#include "ShaderPackPage.h"
#include "ui/dialogs/ModDownloadDialog.h"

ShaderPackPage::ShaderPackPage(MinecraftInstance* instance, std::shared_ptr<ShaderPackFolderModel> model, QWidget* parent)
    : DownloadableResourcesPage(instance, model, parent)
{
    ui->actionViewConfigs->setVisible(false);

    setupDownloadAction(tr("Download Shader Packs"), tr("Download shader packs from online mod platforms"));
    connect(ui->actionDownloadItem, &QAction::triggered, this, &ShaderPackPage::installShaderPacks);
}

void ShaderPackPage::installShaderPacks()
{
    if (!m_controlsEnabled)
        return;
    if (m_instance->typeName() != "Minecraft")
        return;  // this is a null instance or a legacy instance

    ModDownloadDialog mdownload(m_model, this, ModAPI::ShaderPack, m_instance);
    if (mdownload.exec()) {
        runTasks(mdownload.getTasks());
    }
}

// TODO: update
