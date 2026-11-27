#include "ModrinthCheckUpdate.h"
#include "ModrinthAPI.h"
#include "ModrinthPackIndex.h"

#include <QDebug>
#include <QEventLoop>
#include <QJsonDocument>
#include <QMap>
#include <QPair>
#include <QStringList>

#include <utility>

#include "Json.h"
#include "modplatform/ModIndex.h"

#include "ModDownloadTask.h"

#include "modplatform/helpers/HashUtils.h"

#include "tasks/ConcurrentTask.h"

static ModPlatform::ProviderCapabilities ModrinthUpdateProviderCaps;

bool ModrinthCheckUpdate::abort()
{
    if (m_net_job)
    {
        return m_net_job->abort();
    }
    return true;
}

/* Check for update:
 * - Get latest version available
 * - Compare hash of the latest version with the current hash
 * - If equal, no updates, else, there's updates, so add to the list
 * */
void ModrinthCheckUpdate::executeTask()
{
    setStatus(tr("Preparing mods for Modrinth..."));
    setProgress(0, 3);

    QHash<QString, Mod*> mappings;

    // Create all hashes
    QList<QPair<Mod*, Hashing::Hasher::Ptr>> hashers;
    auto best_hash_type =
        ModrinthUpdateProviderCaps.hashType(ModPlatform::Provider::MODRINTH)
            .first();

    ConcurrentTask hashing_task(this, "MakeModrinthHashesTask", 10);
    for (auto* mod : std::as_const(m_mods))
    {
        if (!mod->enabled())
        {
            emit checkFailed(mod, tr("Disabled mods won't be updated, to "
                                     "prevent mod duplication issues!"));
            continue;
        }

        auto hash = mod->metadata()->hash;

        // Sadly the API can only handle one hash type per call, se we
        // need to generate a new hash if the current one is innadequate
        // (though it will rarely happen, if at all)
        if (mod->metadata()->hash_format != best_hash_type)
        {
            auto hash_task = Hashing::createModrinthHasher(
                mod->fileinfo().absoluteFilePath());
            hashers.append({mod, hash_task});
            hashing_task.addTask(hash_task);
        }
        else
        {
            mappings.insert(hash, mod);
        }
    }

    QEventLoop loop;
    connect(&hashing_task, &Task::finished, &loop, &QEventLoop::quit);
    hashing_task.start();
    loop.exec();

    for (const auto& [mod, hasher] : std::as_const(hashers))
    {
        if (hasher->wasSuccessful())
        {
            QString hash(hasher->getResult());
            mappings.insert(hash, mod);
        }
        else if (hasher->getState() == Task::State::Failed)
        {
            emit checkFailed(mod, tr("Failed to generate hash"));
        }
    }

    // A project ID only means something on the source it came from, so ask
    // every source about its own mods
    QMap<QString, QHash<QString, Mod*>> mappings_by_source;
    for (auto it = mappings.cbegin(); it != mappings.cend(); ++it)
    {
        mappings_by_source[it.value()->metadata()->api_url].insert(it.key(),
                                                                   it.value());
    }

    setProgress(1, 3);

    // Collect errors here and report them once at the end, emitting failed()
    // directly would be followed by emitSucceeded()
    QStringList errors;
    for (auto it = mappings_by_source.cbegin(); it != mappings_by_source.cend();
         ++it)
    {
        auto error = checkSource(it.key(), it.value(), best_hash_type);
        if (getState() == Task::State::AbortedByUser)
        {
            return;
        }
        if (!error.isEmpty())
        {
            errors.append(error);
        }
    }

    if (!errors.isEmpty())
    {
        emitFailed(errors.join('\n'));
        return;
    }
    emitSucceeded();
}

QString ModrinthCheckUpdate::checkSource(const QString& url,
                                         const QHash<QString, Mod*>& mappings,
                                         const QString& format)
{
    auto source = Modrinth::findSource(url);
    if (!source)
    {
        for (auto* mod : mappings)
        {
            emit checkFailed(mod,
                             tr("The source %1 is no longer in your Modrinth "
                                "sources.")
                                 .arg(url));
        }
        return {};
    }

    QByteArray response;
    auto job = ModrinthAPI(*source).latestVersions(
        mappings.keys(), format, m_game_versions, m_loaders, &response);

    QEventLoop lock;

    connect(job.get(), &Task::finished, &lock, &QEventLoop::quit);

    setStatus(tr("Waiting for the API response from %1...")
                  .arg(source->displayName()));

    m_net_job = job.get();
    job->start();

    lock.exec();

    m_net_job = nullptr;

    // abort() only aborts the request, report it from here
    if (job->getState() == Task::State::AbortedByUser)
    {
        emitAborted();
        return {};
    }
    if (job->getState() == Task::State::Failed)
    {
        return job->failReason();
    }
    if (job->wasSuccessful())
    {
        return parseLatestVersions(response, mappings, format, *source);
    }
    return {};
}

QString ModrinthCheckUpdate::parseLatestVersions(
    const QByteArray& response, const QHash<QString, Mod*>& mappings,
    const QString& format, const Modrinth::Source& source)
{
    QJsonParseError parse_error{};
    QJsonDocument doc = QJsonDocument::fromJson(response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from "
                      "ModrinthCheckUpdate at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << response;

        return parse_error.errorString();
    }

    setStatus(
        tr("Parsing the API response from %1...").arg(source.displayName()));
    setProgress(2, 3);

    try
    {
        for (auto hash : mappings.keys())
        {
            auto project_obj = doc[hash].toObject();

            // If the returned project is empty, but we have Modrinth
            // metadata, it means this specific version is not available
            if (project_obj.isEmpty())
            {
                qDebug() << "Mod " << mappings.find(hash).value()->name()
                         << " got an empty response.";
                qDebug() << "Hash: " << hash;

                emit checkFailed(mappings.find(hash).value(),
                                 tr("No valid version found for this mod. It's "
                                    "probably unavailable for the current game "
                                    "version / mod loader."));

                continue;
            }

            // Sometimes a version may have multiple files, one with
            // "forge" and one with "fabric", so we may want to filter
            // it
            QString loader_filter;
            static auto flags = {
                ModAPI::ModLoaderType::Forge, ModAPI::ModLoaderType::NeoForge,
                ModAPI::ModLoaderType::Fabric, ModAPI::ModLoaderType::Quilt};
            for (auto flag : flags)
            {
                if (m_loaders.testFlag(flag))
                {
                    loader_filter = ModAPI::getModLoaderString(flag);
                    break;
                }
            }

            // Currently, we rely on a couple heuristics to determine
            // whether an update is actually available or not:
            // - The file needs to be preferred: It is either the
            // primary file, or the one found via (explicit) usage of
            // the loader_filter
            // - The version reported by the JAR is different from the
            // version reported by the indexed version (it's usually the
            // case) Such is the pain of having arbitrary files for a
            // given version .-.

            auto project_ver = Modrinth::loadIndexedPackVersion(
                project_obj, format, loader_filter);
            if (project_ver.downloadUrl.isEmpty())
            {
                qCritical() << "Modrinth mod without download url!";
                qCritical() << project_ver.fileName;

                emit checkFailed(mappings.find(hash).value(),
                                 tr("Mod has an empty download URL"));

                continue;
            }

            auto mod_iter = mappings.find(hash);
            if (mod_iter == mappings.end())
            {
                qCritical() << "Failed to remap mod from Modrinth!";
                continue;
            }
            auto mod = *mod_iter;

            auto key = project_ver.hash;
            if ((key != hash && project_ver.is_preferred) ||
                (mod->status() == ModStatus::NotInstalled))
            {
                if (mod->version() == project_ver.version_number)
                {
                    continue;
                }

                // Fake pack with the necessary info to pass to the
                // download task :)
                ModPlatform::IndexedPack pack;
                pack.name = mod->name();
                pack.slug = mod->metadata()->slug;
                pack.addonId = mod->metadata()->project_id;
                pack.websiteUrl = mod->homeurl();
                for (auto& author : mod->authors())
                {
                    pack.authors.append({author, ""});
                }
                pack.description = mod->description();
                pack.provider = ModPlatform::Provider::MODRINTH;
                pack.apiUrl = source.api_url;

                auto download_task =
                    new ModDownloadTask(pack, project_ver, m_mods_folder);

                m_updatable.emplace_back(
                    pack.name, hash, mod->version(), project_ver.version_number,
                    project_ver.changelog, ModPlatform::Provider::MODRINTH,
                    download_task);
                if (!source.isOfficial())
                {
                    m_updatable.back().source_name = source.displayName();
                }
            }
        }
    }
    catch (Json::JsonException& e)
    {
        return e.cause() + " : " + e.what();
    }

    return {};
}

#include "moc_ModrinthCheckUpdate.cpp"
