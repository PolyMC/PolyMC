#pragma once

#include "Application.h"
#include "minecraft/mod/ModFolderModel.h"
#include "modplatform/CheckUpdateTask.h"
#include "net/NetJob.h"

class ModrinthCheckUpdate : public CheckUpdateTask
{
    Q_OBJECT

public:
    ModrinthCheckUpdate(QList<Mod*>& mods, std::list<Version>& mcVersions,
                        ModAPI::ModLoaderTypes loaders,
                        std::shared_ptr<ModFolderModel> mods_folder)
        : CheckUpdateTask(mods, mcVersions, loaders, mods_folder)
    {
    }

public slots:
    bool abort() override;

protected slots:
    void executeTask() override;

private:
    void onHashFailed(Mod* mod);
    void onLatestVersionsReceived(QByteArray* response,
                                  QHash<QString, Mod*>& mappings,
                                  QString& error, const QString& format,
                                  NetJob::Ptr job);

    NetJob* m_net_job = nullptr;
};
