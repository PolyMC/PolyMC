// SPDX-License-Identifier: GPL-3.0-only

#include "ResolveDependenciesTask.h"

#include <QByteArray>
#include <QDebug>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <utility>

#include "Application.h"
#include "Json.h"
#include "Version.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "minecraft/mod/Mod.h"
#include "modplatform/flame/FlameAPI.h"
#include "modplatform/flame/FlameModIndex.h"
#include "modplatform/modrinth/ModrinthAPI.h"
#include "modplatform/modrinth/ModrinthPackIndex.h"

namespace
{
// Mods needing mods needing mods... stop somewhere, in case of a loop the
// known projects don't catch
constexpr int max_dependency_depth = 10;
constexpr int max_dependencies = 100;
}  // namespace

ResolveDependenciesTask::ResolveDependenciesTask(QList<Selected> selected,
                                                 BaseInstance* instance,
                                                 QList<Mod*> installed)
    : Task(nullptr),
      m_selected(std::move(selected)),
      m_instance(instance),
      m_installed(std::move(installed))
{
}

QString ResolveDependenciesTask::unresolvedText() const
{
    QString text;
    for (const auto& dependency : m_unresolved)
    {
        text += tr("%1 (required by %2)")
                    .arg(dependency.name.toHtmlEscaped(),
                         dependency.required_by.toHtmlEscaped()) +
                "<br>" + dependency.reason.toHtmlEscaped() + "<br>";
        if (!dependency.url.isEmpty())
        {
            text += QString("<a href='%1'>%1</a><br>")
                        .arg(dependency.url.toHtmlEscaped());
        }
        text += "<br>";
    }
    return text;
}

bool ResolveDependenciesTask::abort()
{
    m_stopped = true;
    if (m_current_job)
    {
        m_current_job->abort();
    }
    return true;
}

QString ResolveDependenciesTask::projectKey(ModPlatform::Provider provider,
                                            const QString& source,
                                            const QString& project)
{
    return QString("%1 %2 %3")
        .arg(QString::number(static_cast<int>(provider)), source, project);
}

void ResolveDependenciesTask::executeTask()
{
    setStatus(tr("Resolving dependencies..."));

    auto* instance = dynamic_cast<MinecraftInstance*>(m_instance);
    if (!instance)
    {
        emitSucceeded();
        return;
    }
    auto profile = instance->getPackProfile();
    m_minecraft_version = profile->getComponentVersion("net.minecraft");
    m_loaders = profile->getModLoaders();

    for (auto* mod : std::as_const(m_installed))
    {
        auto metadata = mod->metadata();
        if (metadata)
        {
            m_known.insert(projectKey(metadata->provider, metadata->api_url,
                                      metadata->project_id.toString()));
        }
    }
    for (const auto& selected : std::as_const(m_selected))
    {
        m_known.insert(projectKey(selected.pack.provider, selected.pack.apiUrl,
                                  selected.pack.addonId.toString()));
    }

    for (const auto& selected : std::as_const(m_selected))
    {
        // Even if the source was removed in the meantime, its URL is enough
        // to ask it
        Modrinth::Source source;
        source.api_url = selected.pack.apiUrl;
        if (auto configured = Modrinth::findSource(selected.pack.apiUrl))
        {
            source = *configured;
        }
        queueDependencies(selected.version, selected.pack.provider, source,
                          selected.pack.name, 1);
    }

    while (!m_pending.isEmpty() && !m_stopped)
    {
        auto pending = m_pending.takeFirst();
        if (pending.dependency.type != ModPlatform::DependencyType::Required)
        {
            continue;
        }

        auto target = findTarget(pending);
        if (target)
        {
            resolve(pending, *target);
        }
    }

    if (m_stopped)
    {
        emitAborted();
        return;
    }
    emitSucceeded();
}

void ResolveDependenciesTask::queueDependencies(
    const ModPlatform::IndexedVersion& version, ModPlatform::Provider provider,
    const Modrinth::Source& source, const QString& parent, int depth)
{
    for (const auto& dependency : version.dependencies)
    {
        m_pending.append({dependency, provider, source, parent, depth});
    }
}

void ResolveDependenciesTask::addUnresolved(const Pending& pending,
                                            const QString& reason)
{
    const auto& dependency = pending.dependency;

    auto name = dependency.name;
    if (name.isEmpty())
    {
        name = dependency.addonId.toString();
    }
    if (name.isEmpty())
    {
        name = dependency.external_id;
    }
    if (name.isEmpty())
    {
        name = tr("Unknown dependency");
    }

    m_unresolved.append({name, dependency.url, pending.required_by, reason});
}

std::optional<ResolveDependenciesTask::Target>
ResolveDependenciesTask::findTarget(const Pending& pending)
{
    const auto& dependency = pending.dependency;

    // A Modrinth-compatible source saying the dependency is somewhere else.
    // Following it is fine, unlike looking IDs up on another source.
    if (!dependency.external_source.isEmpty())
    {
        if (dependency.external_id.isEmpty())
        {
            addUnresolved(pending,
                          tr("No %1 project is specified for this dependency.")
                              .arg(dependency.external_source));
            return {};
        }
        if (dependency.external_source == "modrinth")
        {
            return Target{ModPlatform::Provider::MODRINTH, Modrinth::Source{},
                          dependency.external_id, dependency.external_version};
        }
        if (dependency.external_source == "curseforge")
        {
            if (!(APPLICATION->capabilities() & Application::SupportsFlame))
            {
                addUnresolved(
                    pending,
                    tr("Downloading from CurseForge requires an API key."));
                return {};
            }
            return Target{
                ModPlatform::Provider::FLAME, {}, dependency.external_id, {}};
        }

        addUnresolved(pending, tr("Dependencies from %1 are not supported.")
                                   .arg(dependency.external_source));
        return {};
    }

    if (!dependency.addonId.toString().isEmpty() ||
        !dependency.version.isEmpty())
    {
        return Target{pending.provider, pending.source,
                      dependency.addonId.toString(), dependency.version};
    }

    addUnresolved(pending,
                  tr("This dependency is not hosted on %1.")
                      .arg(pending.provider == ModPlatform::Provider::FLAME
                               ? QString("CurseForge")
                               : pending.source.displayName()));
    return {};
}

void ResolveDependenciesTask::resolve(const Pending& pending,
                                      const Target& target)
{
    // Dependencies naming only a version only tell their project once looked
    // up
    auto key = projectKey(target.provider, target.source.api_url,
                          target.project.isEmpty() ? "version:" + target.version
                                                   : target.project);
    if (m_known.contains(key))
    {
        return;
    }
    m_known.insert(key);

    if (pending.depth > max_dependency_depth ||
        m_resolved.size() >= max_dependencies)
    {
        addUnresolved(pending, tr("Too many nested dependencies to resolve."));
        return;
    }

    ModPlatform::IndexedPack pack;
    ModPlatform::IndexedVersion version;
    auto error = target.provider == ModPlatform::Provider::FLAME
                     ? resolveFlame(target, pack, version)
                     : resolveModrinth(target, pack, version);
    if (m_stopped)
    {
        return;
    }
    if (!error.isEmpty())
    {
        if (pending.dependency.name.isEmpty() && !pack.name.isEmpty())
        {
            auto named = pending;
            named.dependency.name = pack.name;
            addUnresolved(named, error);
            return;
        }
        addUnresolved(pending, error);
        return;
    }

    if (target.project.isEmpty())
    {
        auto pack_key = projectKey(target.provider, target.source.api_url,
                                   pack.addonId.toString());
        if (m_known.contains(pack_key))
        {
            return;
        }
        m_known.insert(pack_key);
    }

    qDebug() << "Found dependency" << pack.name << "of" << pending.required_by;
    m_resolved.append({pack, version, pending.required_by});
    queueDependencies(version, target.provider, target.source, pack.name,
                      pending.depth + 1);
}

QString ResolveDependenciesTask::resolveModrinth(
    const Target& target, ModPlatform::IndexedPack& pack,
    ModPlatform::IndexedVersion& version)
{
    ModrinthAPI api(target.source);
    auto platform = target.source.displayName();
    auto project = target.project;

    try
    {
        if (!target.version.isEmpty())
        {
            QByteArray response;
            if (!runJob(api.getVersion(target.version, &response)))
            {
                return tr("Failed to fetch the required version from %1.")
                    .arg(platform);
            }
            auto obj = Json::requireObject(QJsonDocument::fromJson(response));
            version = Modrinth::loadIndexedPackVersion(obj);
            if (version.downloadUrl.isEmpty())
            {
                return tr("The required version has no downloadable file.");
            }
            project = version.addonId.toString();
        }

        QByteArray project_response;
        if (!runJob(api.getProject(project, &project_response)))
        {
            return tr("The project was not found on %1.").arg(platform);
        }
        auto project_obj =
            Json::requireObject(QJsonDocument::fromJson(project_response));
        Modrinth::loadIndexedPack(pack, project_obj, ModAPI::Mod, &api);

        if (!target.version.isEmpty())
        {
            return {};
        }

        QByteArray versions_response;
        if (!runJob(api.getVersions({project,
                                     {Version(m_minecraft_version)},
                                     m_loaders,
                                     ModAPI::Mod},
                                    &versions_response)))
        {
            return tr("Failed to fetch the project's versions from %1.")
                .arg(platform);
        }
        auto versions =
            Json::requireArray(QJsonDocument::fromJson(versions_response));
        Modrinth::loadIndexedPackVersions(pack, versions,
                                          APPLICATION->network(), m_instance);
    }
    catch (const Json::JsonException& e)
    {
        qWarning() << "Couldn't read a dependency from" << platform << ":"
                   << e.cause();
        return tr("%1 returned an invalid response.").arg(platform);
    }

    // Newest first
    for (const auto& candidate : std::as_const(pack.versions))
    {
        if (isCompatible(candidate, ModPlatform::Provider::MODRINTH))
        {
            version = candidate;
            return {};
        }
    }
    return tr("No version is compatible with Minecraft %1 and this "
              "instance's mod loader.")
        .arg(m_minecraft_version);
}

QString ResolveDependenciesTask::resolveFlame(
    const Target& target, ModPlatform::IndexedPack& pack,
    ModPlatform::IndexedVersion& version)
{
    FlameAPI api;

    try
    {
        QByteArray project_response;
        if (!runJob(api.getProject(target.project, &project_response)))
        {
            return tr("The project was not found on CurseForge.");
        }
        auto project_obj = Json::requireObject(
            Json::requireObject(QJsonDocument::fromJson(project_response)),
            "data");
        FlameMod::loadIndexedPack(pack, project_obj);

        QByteArray versions_response;
        if (!runJob(api.getVersions({target.project,
                                     {Version(m_minecraft_version)},
                                     m_loaders,
                                     ModAPI::Mod},
                                    &versions_response)))
        {
            return tr(
                "Failed to fetch the project's versions from CurseForge.");
        }
        auto versions = Json::requireArray(
            Json::requireObject(QJsonDocument::fromJson(versions_response)),
            "data");
        FlameMod::loadIndexedPackVersions(pack, versions,
                                          APPLICATION->network(), m_instance);
    }
    catch (const Json::JsonException& e)
    {
        qWarning() << "Couldn't read a dependency from CurseForge:"
                   << e.cause();
        return tr("CurseForge returned an invalid response.");
    }

    bool blocked = false;
    for (const auto& candidate : std::as_const(pack.versions))
    {
        if (!candidate.mcVersion.contains(m_minecraft_version))
        {
            continue;
        }
        // Its author doesn't allow other launchers to download it
        if (candidate.downloadUrl.isEmpty())
        {
            blocked = true;
            continue;
        }
        version = candidate;
        return {};
    }

    if (blocked)
    {
        return tr(
            "The author has disabled downloads from third-party launchers.");
    }
    return tr("No version is compatible with Minecraft %1 and this "
              "instance's mod loader.")
        .arg(m_minecraft_version);
}

bool ResolveDependenciesTask::isCompatible(
    const ModPlatform::IndexedVersion& version,
    ModPlatform::Provider provider) const
{
    if (!version.mcVersion.contains(m_minecraft_version) ||
        version.downloadUrl.isEmpty())
    {
        return false;
    }
    if (provider == ModPlatform::Provider::FLAME)
    {
        return true;
    }

    auto loaders = ModrinthAPI::getModLoaderStrings(m_loaders);
    for (const auto& loader : version.loaders)
    {
        if (loaders.contains(loader))
        {
            return true;
        }
    }
    return false;
}

bool ResolveDependenciesTask::runJob(const NetJob::Ptr& job)
{
    if (!job || m_stopped)
    {
        return false;
    }

    m_current_job = job;

    QEventLoop loop;
    connect(job.get(), &Task::finished, &loop, &QEventLoop::quit);
    job->start();
    if (!job->isFinished())
    {
        loop.exec();
    }

    m_current_job.reset();
    return job->wasSuccessful();
}

#include "moc_ResolveDependenciesTask.cpp"
