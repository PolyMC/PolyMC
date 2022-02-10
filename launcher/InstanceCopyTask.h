#pragma once

#include "BaseInstance.h"
#include "BaseVersion.h"
#include "InstanceTask.h"
#include "net/NetJob.h"
#include "settings/SettingsObject.h"
#include "tasks/Task.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QUrl>

class InstanceCopyTask : public InstanceTask
{
    Q_OBJECT
public:
    explicit InstanceCopyTask(InstancePtr origInstance, bool copySaves, bool keepPlaytime);

protected:
    //! Entry point for tasks.
    void executeTask() override;
    void copyFinished();
    void copyAborted();

private: /* data */
    InstancePtr m_origInstance;
    QFuture<bool> m_copyFuture;
    QFutureWatcher<bool> m_copyFutureWatcher;
    std::unique_ptr<IPathMatcher> m_matcher;
    bool m_keepPlaytime;
};
