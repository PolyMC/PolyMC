#pragma once

#include "PackManifest.h"
#include "net/NetJob.h"
#include "tasks/Task.h"

namespace Flame
{
class FileResolvingTask : public Task
{
    Q_OBJECT
public:
    explicit FileResolvingTask(shared_qobject_ptr<QNetworkAccessManager> network, Flame::Manifest &toProcess);
    ~FileResolvingTask() override = default;

    const Flame::Manifest &getResults() const
    {
        return m_toProcess;
    }

protected:
    void executeTask() override;

protected slots:
    void netJobFinished();

private: /* data */
    shared_qobject_ptr<QNetworkAccessManager> m_network;
    Flame::Manifest m_toProcess;
    QVector<QByteArray> results;
    NetJob::Ptr m_dljob;
};
}
