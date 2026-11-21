#pragma once

#include <QList>

#include <list>
#include <memory>

#include "Version.h"
#include "minecraft/mod/Mod.h"
#include "minecraft/mod/ModFolderModel.h"
#include "modplatform/CheckUpdateTask.h"
#include "modplatform/ModAPI.h"
#include "net/NetJob.h"

class FlameCheckUpdate : public CheckUpdateTask
{
    Q_OBJECT

public:
    FlameCheckUpdate(QList<Mod*>& mods, std::list<Version>& mcVersions,
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
    bool runJob(NetJob::Ptr job);

    NetJob* m_net_job = nullptr;

    bool m_was_aborted = false;
};
