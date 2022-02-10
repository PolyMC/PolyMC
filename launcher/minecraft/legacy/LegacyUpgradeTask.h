#pragma once

#include "BaseInstance.h"
#include "BaseVersion.h"
#include "InstanceTask.h"
#include "net/NetJob.h"
#include "settings/SettingsObject.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QUrl>


class LegacyUpgradeTask : public InstanceTask
{
    Q_OBJECT
public:
    explicit LegacyUpgradeTask(InstancePtr origInstance);

protected:
    //! Entry point for tasks.
    void executeTask() override;
    void copyFinished();
    void copyAborted();

private: /* data */
    InstancePtr m_origInstance;
    QFuture<bool> m_copyFuture;
    QFutureWatcher<bool> m_copyFutureWatcher;
};
