#pragma once
#include "tasks/Task.h"
#include "net/NetJob.h"
#include "minecraft/Library.h"
class MinecraftInstance;

class LibrariesTask : public Task
{
    Q_OBJECT
public:
    LibrariesTask(MinecraftInstance* inst);
    virtual ~LibrariesTask() {};

    void executeTask() override;

    bool canAbort() const override;

private slots:
    void jarlibFailed(QString reason);

public slots:
    bool abort() override;

private:
    bool processArtifactPool(MinecraftInstance* inst, HttpMetaCache* cache,
                             const QList<LibraryPtr>& pool, QStringList& errors,
                             const QString& path);
    void downloadAborted();

    MinecraftInstance* m_inst;
    NetJob::Ptr downloadJob;
};
