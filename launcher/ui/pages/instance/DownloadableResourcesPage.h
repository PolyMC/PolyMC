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

#include "ExternalResourcesPage.h"
#include "ModDownloadTask.h"

class DownloadableResourcesPage : public ExternalResourcesPage {
    Q_OBJECT
public:
    DownloadableResourcesPage(BaseInstance* instance,
                              std::shared_ptr<ResourceFolderModel> model,
                              QWidget* parent = nullptr);

protected:
    // run the given download tasks
    void runTasks(const QList<ModDownloadTask*>& toRun);
};
