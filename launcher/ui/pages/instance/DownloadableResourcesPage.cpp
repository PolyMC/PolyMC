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
#include "DownloadableResourcesPage.h"
#include "minecraft/mod/ResourceFolderModel.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ProgressDialog.h"
#include "ui/pages/instance/ExternalResourcesPage.h"

DownloadableResourcesPage::DownloadableResourcesPage(BaseInstance* instance,
                                                     std::shared_ptr<ResourceFolderModel> model,
                                                     QWidget* parent)
    : ExternalResourcesPage(instance, model, parent) {}

void DownloadableResourcesPage::runTasks(const QList<ModDownloadTask*>& toRun) {
    auto* tasks = new ConcurrentTask(this);
    connect(tasks, &Task::failed, [this, tasks](QString reason) {
        CustomMessageBox::selectable(this, tr("Error"), reason, QMessageBox::Critical)->show();
        tasks->deleteLater();
    });

    connect(tasks, &Task::aborted, [this, tasks]() {
        CustomMessageBox::selectable(this, tr("Aborted"), tr("Download stopped by user."),
                                     QMessageBox::Information)
            ->show();
        tasks->deleteLater();
    });

    connect(tasks, &Task::succeeded, [this, tasks]() {
        const auto warnings = tasks->warnings();
        if (warnings.count())
            CustomMessageBox::selectable(this, tr("Warnings"), warnings.join('\n'),
                                         QMessageBox::Warning)
                ->show();
        tasks->deleteLater();
    });

    for (auto* task : toRun)
        tasks->addTask(task);

    ProgressDialog loadDialog(this);
    loadDialog.setSkipButton(true, tr("Abort"));
    loadDialog.execWithTask(tasks);

    m_model->update();
}
