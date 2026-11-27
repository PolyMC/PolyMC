// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QList>
#include <QSet>
#include <QString>

#include <optional>

#include "modplatform/ModAPI.h"
#include "modplatform/ModIndex.h"
#include "modplatform/modrinth/ModrinthSource.h"
#include "net/NetJob.h"
#include "tasks/Task.h"

class BaseInstance;
class Mod;

// Finds the required dependencies of mods about to be installed, and theirs in
// turn, that aren't installed or picked already. A dependency is looked up on
// the platform and Modrinth source of the mod needing it, or wherever a
// Modrinth-compatible source says it is.
class ResolveDependenciesTask : public Task
{
    Q_OBJECT

public:
    struct Selected
    {
        ModPlatform::IndexedPack pack;
        ModPlatform::IndexedVersion version;
    };

    struct Resolved
    {
        ModPlatform::IndexedPack pack;
        ModPlatform::IndexedVersion version;
        QString required_by;
    };

    // Dependencies to install by hand, if at all
    struct Unresolved
    {
        QString name;
        QString url;
        QString required_by;
        QString reason;
    };

    ResolveDependenciesTask(QList<Selected> selected, BaseInstance* instance,
                            QList<Mod*> installed);

    const QList<Resolved>& resolved() const
    {
        return m_resolved;
    }
    const QList<Unresolved>& unresolved() const
    {
        return m_unresolved;
    }
    // The unresolved dependencies as HTML, with their links
    QString unresolvedText() const;

    bool canAbort() const override
    {
        return true;
    }
    bool abort() override;

protected:
    void executeTask() override;

private:
    struct Pending
    {
        ModPlatform::Dependency dependency;
        ModPlatform::Provider provider;
        Modrinth::Source source;
        QString required_by;
        int depth;
    };

    struct Target
    {
        ModPlatform::Provider provider;
        Modrinth::Source source;
        QString project;
        QString version;
    };

    void queueDependencies(const ModPlatform::IndexedVersion& version,
                           ModPlatform::Provider provider,
                           const Modrinth::Source& source,
                           const QString& parent, int depth);
    std::optional<Target> findTarget(const Pending& pending);
    void resolve(const Pending& pending, const Target& target);
    QString resolveModrinth(const Target& target,
                            ModPlatform::IndexedPack& pack,
                            ModPlatform::IndexedVersion& version);
    QString resolveFlame(const Target& target, ModPlatform::IndexedPack& pack,
                         ModPlatform::IndexedVersion& version);
    bool isCompatible(const ModPlatform::IndexedVersion& version,
                      ModPlatform::Provider provider) const;
    bool runJob(const NetJob::Ptr& job);
    void addUnresolved(const Pending& pending, const QString& reason);

    static QString projectKey(ModPlatform::Provider provider,
                              const QString& source, const QString& project);

    QList<Selected> m_selected;
    BaseInstance* m_instance;
    QList<Mod*> m_installed;

    QString m_minecraft_version;
    ModAPI::ModLoaderTypes m_loaders;

    QList<Pending> m_pending;
    // Projects installed, picked or looked at already
    QSet<QString> m_known;

    QList<Resolved> m_resolved;
    QList<Unresolved> m_unresolved;

    NetJob::Ptr m_current_job;
    bool m_stopped = false;
};
