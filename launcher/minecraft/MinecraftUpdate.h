/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QObject>
#include <QList>
#include <QString>

#include "tasks/Task.h"

class MinecraftVersion;
class MinecraftInstance;

// FIXME: This looks very similar to a SequentialTask. Maybe we can reduce code
// duplications? :^)

class MinecraftUpdate : public Task
{
    Q_OBJECT
public:
    // Launching resolves the components first, then picks Java, then
    // downloads the files: which natives a library needs depends on the
    // architecture of that Java
    enum class Part
    {
        All,
        Components,
        Files
    };

    explicit MinecraftUpdate(MinecraftInstance* inst, Part part = Part::All,
                             QObject* parent = 0);
    virtual ~MinecraftUpdate() {};

    void executeTask() override;
    bool canAbort() const override;

private slots:
    bool abort() override;
    void subtaskSucceeded();
    void subtaskFailed(QString error);

private:
    void next();

private:
    MinecraftInstance* m_inst = nullptr;
    Part m_part = Part::All;
    QList<Task::Ptr> m_tasks;
    QString m_preFailure;
    int m_currentTask = -1;
    bool m_abort = false;
    bool m_failed_out_of_order = false;
    QString m_fail_reason;
};
