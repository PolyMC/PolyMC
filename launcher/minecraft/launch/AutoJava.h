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

#pragma once

#include <launch/LaunchStep.h>

#include "java/download/JavaRuntimeTasks.h"

/**
 * Picks a launcher-managed Java runtime that fits the instance, downloading one if needed.
 * Does nothing when automatic Java is turned off or the instance sets its own Java.
 */
class AutoJava : public LaunchStep {
    Q_OBJECT
   public:
    explicit AutoJava(LaunchTask* parent) : LaunchStep(parent) {}

    void executeTask() override;
    void proceed() override;
    bool canAbort() const override;

   public slots:
    bool abort() override;

   private:
    void installFinished();
    void use(const JavaRuntime::InstalledRuntime& runtime);
    void fallBack(const QString& reason);

    shared_qobject_ptr<JavaRuntime::InstallTask> m_installTask;
};
