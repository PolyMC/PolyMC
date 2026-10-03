#pragma once

#include "ModAPI.h"
#include "Version.h"
#include "modplatform/ModIndex.h"
#include "modplatform/flame/FlameAPI.h"
#include "modplatform/modrinth/ModrinthAPI.h"
#include "tasks/Task.h"

class ModDependencyTask : public Task {
    Q_OBJECT
public:
    struct Root {
        ModPlatform::Provider prov;
        QVariant addonId;
        QList<ModPlatform::Dependency> deps;
    };
    struct Resolved {
        ModPlatform::IndexedPack pack;
        ModPlatform::IndexedVersion ver;
    };

    explicit ModDependencyTask(const QList<Root>& roots, const std::list<Version>& mcVersions,
                               ModAPI::ModLoaderTypes loaders, ModAPI::ResourceType type,
                               QObject* parent = nullptr);

    const QList<Resolved>& resolved() const {
        return m_resolved;
    }

    const QStringList& unresolved() const {
        return m_unresolved;
    }

    bool abort() override
    {
        m_aborted = true;
        return true;
    }

    static ModAPI* apiFor (ModPlatform::Provider p) {
        static FlameAPI flameApi;
        static ModrinthAPI modrinthApi;

        return p == ModPlatform::Provider::FLAME
                   ? static_cast<ModAPI*>(&flameApi)
                   : static_cast<ModAPI*>(&modrinthApi);
    };


protected slots:
    void executeTask() override;

private:
    struct Job {
        ModPlatform::Provider prov;
        ModPlatform::Dependency dep;
        std::size_t depth = 0;
    };
    static auto key(ModPlatform::Provider p, const QVariant& id) -> QString;
    ModPlatform::IndexedPack fetchPack(const Job& job);

    QList<Root> m_roots;
    std::list<Version> m_mcVersions;
    ModAPI::ModLoaderTypes m_loaders;
    ModAPI::ResourceType m_type;

    QList<Resolved> m_resolved;
    QStringList m_unresolved;
    bool m_aborted = false;
};
