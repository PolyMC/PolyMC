#pragma once

#include <QDir>
#include <QHash>
#include <QList>
#include <QString>
#include <QUrl>
#include <memory>
#include <tuple>

#include "BaseInstance.h"
#include "ModDownloadTask.h"
#include "ReviewMessageBox.h"

#include "minecraft/mod/ModFolderModel.h"

#include "modplatform/CheckUpdateTask.h"
#include "modplatform/ModAPI.h"
#include "modplatform/ModIndex.h"

class Mod;
class ModrinthCheckUpdate;
class FlameCheckUpdate;
class ConcurrentTask;
class Task;

class ModUpdateDialog final : public ReviewMessageBox
{
    Q_OBJECT
public:
    explicit ModUpdateDialog(QWidget* parent, ModAPI::ResourceType type,
                             BaseInstance* instance,
                             const std::shared_ptr<ModFolderModel> mod_model,
                             QList<Mod*>& search_for, bool update_mods = true);
    ~ModUpdateDialog() override
    {
        qDeleteAll(m_tasks);
    }

    void checkCandidates();

    void appendMod(const CheckUpdateTask::UpdatableMod& info);

    const QList<ModDownloadTask*> getTasks();
    QDir indexDir() const
    {
        return m_mod_model->indexDir();
    }

    bool noUpdates() const
    {
        return m_no_updates;
    };
    bool aborted() const
    {
        return m_aborted;
    };

    bool ensureMetadata();

private slots:
    void onMetadataEnsured(Mod*);
    void onMetadataFailed(Mod*);
    void onFirstMetadataFailed(Mod* mod);

private:
    void onCheckFailed(Mod* mod, QString reason, QUrl recovery);
    void onCheckTaskFailed(QString reason);
    // Adds the new dependencies of the updates, returns false to abort
    bool resolveUpdateDependencies();

    QWidget* m_parent;

    ModrinthCheckUpdate* m_modrinth_check_task = nullptr;
    FlameCheckUpdate* m_flame_check_task = nullptr;

    const std::shared_ptr<ModFolderModel> m_mod_model;

    QList<Mod*>& m_candidates;
    QList<Mod*> m_modrinth_to_update;
    QList<Mod*> m_flame_to_update;
    QHash<QString, ModPlatform::Provider> m_fallback_provider;

    ConcurrentTask* m_second_try_metadata;
    QList<std::tuple<Mod*, QString>> m_failed_metadata;
    QList<std::tuple<Mod*, QString, QUrl>> m_failed_check_update;

    QHash<QString, ModDownloadTask*> m_tasks;
    ModAPI::ResourceType m_type;
    QString m_typeString;
    BaseInstance* m_instance;

    bool m_no_updates = false;
    bool m_aborted = false;

    bool m_update_mods = true;
};
