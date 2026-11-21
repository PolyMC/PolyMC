#pragma once

#include "tasks/Task.h"
#include "net/NetJob.h"
#include "PackManifest.h"

namespace Flame
{
class FileResolvingTask : public Task
{
    Q_OBJECT
public:
    explicit FileResolvingTask(
        const shared_qobject_ptr<QNetworkAccessManager>& network,
        Flame::Manifest& toProcess);
    virtual ~FileResolvingTask() {};

    bool canAbort() const override
    {
        return true;
    }
    bool abort() override;

    const Flame::Manifest& getResults() const
    {
        return m_toProcess;
    }

protected:
    virtual void executeTask() override;

protected slots:
    void netJobFinished();

private: /* data */
    shared_qobject_ptr<QNetworkAccessManager> m_network;
    Flame::Manifest m_toProcess;
    std::shared_ptr<QByteArray> result;
    NetJob::Ptr m_dljob;

    void modrinthCheckFinished();
    void onModrinthJobFinished(NetJob* job);
    void onSlugJobFinished(const QVector<QByteArray>& slugs, NetJob* job,
                           QList<File*>* block);

    QMap<File*, QByteArray*> blockedProjects;
};
}  // namespace Flame
