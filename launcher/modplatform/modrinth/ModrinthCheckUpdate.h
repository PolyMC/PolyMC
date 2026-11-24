#pragma once

#include "Version.h"
#include "minecraft/mod/Mod.h"
#include "minecraft/mod/ModFolderModel.h"
#include "modplatform/CheckUpdateTask.h"
#include "modplatform/ModAPI.h"
#include "modplatform/modrinth/ModrinthSource.h"
#include "net/NetJob.h"

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QString>

#include <list>
#include <memory>

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
    QString checkSource(const QString& url,
                        const QHash<QString, Mod*>& mappings,
                        const QString& format);
    QString parseLatestVersions(const QByteArray& response,
                                const QHash<QString, Mod*>& mappings,
                                const QString& format,
                                const Modrinth::Source& source);

    NetJob* m_net_job = nullptr;
};
