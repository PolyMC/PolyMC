#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <memory>
#include "BaseInstance.h"
#include "InstanceTask.h"
#include "pathmatcher/IPathMatcher.h"

class InstanceCopyTask : public InstanceTask
{
    Q_OBJECT
public:
    explicit InstanceCopyTask(InstancePtr origInstance, bool copySaves,
                              bool keepPlaytime);

protected:
    //! Entry point for tasks.
    virtual void executeTask() override;
    void copyFinished();
    void copyAborted();

private: /* data */
    InstancePtr m_origInstance;
    QFuture<bool> m_copyFuture;
    QFutureWatcher<bool> m_copyFutureWatcher;
    std::unique_ptr<IPathMatcher> m_matcher;
    bool m_keepPlaytime;
};
