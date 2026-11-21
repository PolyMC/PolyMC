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
#include "tasks/ConcurrentTask.h"
#include "tasks/Task.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ProgressDialog.h"

#include <QMessageBox>

DownloadableResourcesPage::DownloadableResourcesPage(
    BaseInstance* instance, std::shared_ptr<ResourceFolderModel> model,
    QWidget* parent)
    : ExternalResourcesPage(instance, model, parent)
{
}

void DownloadableResourcesPage::runTasks(const QList<ModDownloadTask*>& toRun)
{
    ConcurrentTask tasks(this);
    for (auto* task : toRun)
    {
        tasks.addTask(task);
    }

    ProgressDialog loadDialog(this);
    loadDialog.setSkipButton(true, tr("Abort"));
    loadDialog.execWithTask(&tasks);

    if (tasks.wasSuccessful())
    {
        const auto warnings = tasks.warnings();
        if (warnings.count())
        {
            CustomMessageBox::selectable(
                this, tr("Warnings"), warnings.join('\n'), QMessageBox::Warning)
                ->show();
        }
    }
    else if (tasks.getState() == Task::State::AbortedByUser)
    {
        CustomMessageBox::selectable(this, tr("Aborted"),
                                     tr("Download stopped by user."),
                                     QMessageBox::Information)
            ->show();
    }
    else
    {
        CustomMessageBox::selectable(this, tr("Error"), tasks.failReason(),
                                     QMessageBox::Critical)
            ->show();
    }

    m_model->update();
}
