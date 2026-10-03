#pragma once

#include "ModIndex.h"
#include "net/NetJob.h"

#include "modplatform/helpers/HashUtils.h"

#include "tasks/ConcurrentTask.h"

class Mod;
class QDir;

class EnsureMetadataTask : public Task {
    Q_OBJECT

   public:
    EnsureMetadataTask(Mod*, QDir, ModPlatform::Provider = ModPlatform::Provider::MODRINTH);
    EnsureMetadataTask(QList<Mod*>&, QDir, ModPlatform::Provider = ModPlatform::Provider::MODRINTH);

    ~EnsureMetadataTask() = default;

    Task::Ptr getHashingTask() { return m_hashing_task; }

   public slots:
    bool abort() override;
   protected slots:
    void executeTask() override;

   private:
    // FIXME: Move to their own namespace
    NetJob::Ptr modrinthVersionsTask();
    NetJob::Ptr modrinthProjectsTask();

    NetJob::Ptr flameVersionsTask();
    NetJob::Ptr flameProjectsTask();

    // Helpers
    enum class RemoveFromList {
        Yes,
        No
    };
    void emitReady(Mod*, QString key = {}, RemoveFromList = RemoveFromList::Yes);
    void emitFail(Mod*, QString key = {}, RemoveFromList = RemoveFromList::Yes);

    // Hashes and stuff
    Hashing::Hasher::Ptr createNewHash(Mod*);
    QString getExistingHash(Mod*);

   private slots:
    void modrinthCallback(ModPlatform::IndexedPack& pack, ModPlatform::IndexedVersion& ver, Mod*);
    void flameCallback(ModPlatform::IndexedPack& pack, ModPlatform::IndexedVersion& ver, Mod*);

   signals:
    void metadataReady(Mod*);
    void metadataFailed(Mod*);

   private:
    QHash<QString, Mod*> m_mods;
    QDir m_index_dir;
    ModPlatform::Provider m_provider;

    QHash<QString, ModPlatform::IndexedVersion> m_temp_versions;
    ConcurrentTask* m_hashing_task;
    NetJob* m_current_task;
};
