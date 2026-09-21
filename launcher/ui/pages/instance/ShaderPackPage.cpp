// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
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

#include "ShaderPackPage.h"
#include "ui_ExternalResourcesPage.h"

#include <QMessageBox>

#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ModDownloadDialog.h"
#include "ui/dialogs/ProgressDialog.h"

#include "tasks/ConcurrentTask.h"

ShaderPackPage::ShaderPackPage(MinecraftInstance* instance, std::shared_ptr<ShaderPackFolderModel> model, QWidget* parent)
    : ExternalResourcesPage(instance, model, parent), m_spModel(model)
{
    ui->actionViewConfigs->setVisible(false);

    ui->actionDownloadItem->setText(tr("Download Packs"));
    ui->actionDownloadItem->setToolTip(tr("Download shader packs from online platforms"));
    ui->actionDownloadItem->setEnabled(true);
    ui->actionAddItem->setText(tr("Add file"));
    ui->actionAddItem->setToolTip(tr("Add a locally downloaded file"));

    ui->actionsToolbar->insertActionBefore(ui->actionAddItem, ui->actionDownloadItem);

    connect(ui->actionDownloadItem, &QAction::triggered, this, &ShaderPackPage::installShaderPacks);
}

void ShaderPackPage::installShaderPacks()
{
    if (!m_controlsEnabled)
        return;
    if (m_instance->typeName() != "Minecraft")
        return;

    ModDownloadDialog dialog(m_spModel, this, m_instance, ModAPI::ShaderPack);
    if (dialog.exec()) {
        ConcurrentTask* tasks = new ConcurrentTask(this);
        connect(tasks, &Task::failed, [this, tasks](QString reason) {
            CustomMessageBox::selectable(this, tr("Error"), reason, QMessageBox::Critical)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::aborted, [this, tasks]() {
            CustomMessageBox::selectable(this, tr("Aborted"), tr("Download stopped by user."), QMessageBox::Information)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::succeeded, [this, tasks]() {
            QStringList warnings = tasks->warnings();
            if (warnings.count())
                CustomMessageBox::selectable(this, tr("Warnings"), warnings.join('\n'), QMessageBox::Warning)->show();

            tasks->deleteLater();
        });

        for (auto& task : dialog.getTasks()) {
            tasks->addTask(task);
        }

        ProgressDialog loadDialog(this);
        loadDialog.setSkipButton(true, tr("Abort"));
        loadDialog.execWithTask(tasks);

        m_model->update();
    }
}
