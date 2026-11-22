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

#include "AutoJava.h"

#include <launch/LaunchTask.h>

#include "Application.h"
#include "FileSystem.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"

void AutoJava::executeTask()
{
    auto instance =
        std::dynamic_pointer_cast<MinecraftInstance>(m_parent->instance());
    auto settings = instance->settings();
    auto global = APPLICATION->settings();

    if (!global->get("AutomaticJava").toBool() ||
        settings->get("OverrideJava").toBool() ||
        settings->get("OverrideJavaLocation").toBool())
    {
        emitSucceeded();
        return;
    }

    auto profile = instance->getPackProfile()->getProfile();
    auto majors = profile ? profile->getCompatibleJavaMajors() : QList<int>();
    if (majors.isEmpty())
    {
        emit logLine(tr("This instance does not say which Java it needs, using "
                        "the Java from the settings.\n"),
                     MessageLevel::Launcher);
        emitSucceeded();
        return;
    }

    auto provider = JavaRuntime::providerFromId(
                        global->get("AutomaticJavaProvider").toString())
                        .value_or(JavaRuntime::Provider::Mojang);
    if (auto runtime = JavaRuntime::findInstalled(majors, provider))
    {
        use(*runtime);
        emitSucceeded();
        return;
    }

    emit logLine(tr("No installed Java fits this instance, downloading Java %1 "
                    "from %2.\n")
                     .arg(majors.first())
                     .arg(JavaRuntime::providerName(provider)),
                 MessageLevel::Launcher);
    m_installTask.reset(new JavaRuntime::InstallTask(provider, majors));
    connect(m_installTask.get(), &Task::finished, this,
            &AutoJava::installFinished);
    connect(m_installTask.get(), &Task::progress, this, &Task::setProgress);
    connect(m_installTask.get(), &Task::status, this, &Task::setStatus);
    emit progressReportingRequest();
}

void AutoJava::proceed()
{
    if (m_installTask)
    {
        m_installTask->start();
    }
}

void AutoJava::installFinished()
{
    auto task = m_installTask;
    m_installTask.reset();

    if (task->getState() == Task::State::AbortedByUser)
    {
        emitFailed(tr("Java download aborted."));
        return;
    }
    if (!task->wasSuccessful())
    {
        fallBack(tr("Could not download Java: %1").arg(task->failReason()));
        return;
    }
    use(task->result());
    emitSucceeded();
}

void AutoJava::use(const JavaRuntime::InstalledRuntime& runtime)
{
    m_parent->instance()->setRuntimeJavaPath(runtime.javaPath);
    emit logLine(tr("Using %1 (Java %2), selected automatically.\n")
                     .arg(runtime.displayName())
                     .arg(runtime.major),
                 MessageLevel::Launcher);
}

void AutoJava::fallBack(const QString& reason)
{
    emit logLine(reason, MessageLevel::Error);

    auto configured = APPLICATION->settings()->get("JavaPath").toString();
    if (!configured.isEmpty() && !FS::ResolveExecutable(configured).isEmpty())
    {
        emit logLine(tr("Falling back to the Java from the settings: %1\n")
                         .arg(configured),
                     MessageLevel::Warning);
        emitSucceeded();
        return;
    }
    emit logLine(tr("Set up Java in the settings, or turn off automatic Java "
                    "there and choose one yourself.\n"),
                 MessageLevel::Launcher);
    emitFailed(reason);
}

bool AutoJava::canAbort() const
{
    return !m_installTask || m_installTask->canAbort();
}

bool AutoJava::abort()
{
    if (m_installTask)
    {
        return m_installTask->abort();
    }
    return true;
}
