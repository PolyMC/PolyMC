#pragma once

#include "BaseVersion.h"
#include "InstanceTask.h"
#include "net/NetJob.h"
#include "settings/SettingsObject.h"
#include "tasks/Task.h"
#include <QUrl>

class InstanceCreationTask : public InstanceTask
{
    Q_OBJECT
public:
    explicit InstanceCreationTask(BaseVersionPtr version);

protected:
    //! Entry point for tasks.
    void executeTask() override;

private: /* data */
    BaseVersionPtr m_version;
};
