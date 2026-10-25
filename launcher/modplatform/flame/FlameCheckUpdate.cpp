#include "FlameCheckUpdate.h"
#include "FlameAPI.h"
#include "FlameModIndex.h"

#include <MurmurHash2.h>

#include "FileSystem.h"
#include "Json.h"

#include "ModDownloadTask.h"

#include <memory>
#include <vector>

static FlameAPI api;

bool FlameCheckUpdate::abort()
{
    m_was_aborted = true;
    if (m_net_job)
        return m_net_job->abort();
    return true;
}

ModPlatform::IndexedPack getProjectInfo(ModPlatform::IndexedVersion& ver_info)
{
    ModPlatform::IndexedPack pack;

    QEventLoop loop;

    auto get_project_job = new NetJob("Flame::GetProjectJob", APPLICATION->network());

    auto response = new QByteArray();
    auto url = QString("https://api.curseforge.com/v1/mods/%1").arg(ver_info.addonId.toString());
    auto dl = Net::Download::makeByteArray(url, response);
    get_project_job->addNetAction(dl);

    QObject::connect(get_project_job, &NetJob::succeeded, [response, &pack]() {
        QJsonParseError parse_error{};
        QJsonDocument doc = QJsonDocument::fromJson(*response, &parse_error);
        if (parse_error.error != QJsonParseError::NoError) {
            qWarning() << "Error while parsing JSON response from FlameCheckUpdate at " << parse_error.offset
                       << " reason: " << parse_error.errorString();
            qWarning() << *response;
            return;
        }

        try {
            auto doc_obj = Json::requireObject(doc);
            auto data_obj = Json::requireObject(doc_obj, "data");
            FlameMod::loadIndexedPack(pack, data_obj);
        } catch (Json::JsonException& e) {
            qWarning() << e.cause();
            qDebug() << doc;
        }
    });

    QObject::connect(get_project_job, &NetJob::finished, [&loop, get_project_job, response] {
        get_project_job->deleteLater();
        delete response;
        loop.quit();
    });

    get_project_job->start();
    loop.exec();

    return pack;
}

static QString getFileInfoURL(int addonId, int fileId)
{
    return QString("https://api.curseforge.com/v1/mods/%1/files/%2").arg(QString::number(addonId), QString::number(fileId));
}

static ModPlatform::IndexedVersion parseFileInfo(const QByteArray& response)
{
    ModPlatform::IndexedVersion ver;

    QJsonParseError parse_error{};
    QJsonDocument doc = QJsonDocument::fromJson(response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError) {
        qWarning() << "Error while parsing JSON response from FlameCheckUpdate at " << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << response;
        return ver;
    }

    try {
        auto doc_obj = Json::requireObject(doc);
        auto data_obj = Json::requireObject(doc_obj, "data");
        ver = FlameMod::loadIndexedPackVersion(data_obj);
    } catch (Json::JsonException& e) {
        qWarning() << e.cause();
        qDebug() << doc;
    }

    return ver;
}

bool FlameCheckUpdate::runJob(NetJob::Ptr job)
{
    QEventLoop loop;
    connect(job.get(), &Task::finished, &loop, &QEventLoop::quit);
    connect(job.get(), &Task::progress, this, &FlameCheckUpdate::setProgress);

    m_net_job = job.get();
    job->start();
    loop.exec();
    m_net_job = nullptr;

    return !m_was_aborted;
}

/* Check for update:
 * - Get latest version available
 * - Compare hash of the latest version with the current hash
 * - If equal, no updates, else, there's updates, so add to the list
 * */
void FlameCheckUpdate::executeTask()
{
    setStatus(tr("Preparing mods for CurseForge..."));

    auto succeededData = [](const Net::Download::Ptr& download, const std::shared_ptr<QByteArray>& data) {
        return download && download->getState() == Task::State::Succeeded ? *data : QByteArray();
    };

    struct VersionCheck {
        Mod* mod;
        std::shared_ptr<QByteArray> response;
        Net::Download::Ptr download;
    };
    std::vector<VersionCheck> checks;

    NetJob::Ptr versions_job = new NetJob("Flame::CheckUpdate", APPLICATION->network());
    for (auto* mod : std::as_const(m_mods)) {
        if (!mod->enabled()) {
            emit checkFailed(mod, tr("Disabled mods won't be updated, to prevent mod duplication issues!"));
            continue;
        }

        VersionCheck check{ mod, std::make_shared<QByteArray>(), nullptr };
        ModAPI::VersionSearchArgs args{ mod->metadata()->project_id.toString(), m_game_versions, m_loaders };
        check.download = Net::Download::makeByteArray(api.getLatestVersionURL(args), check.response.get());
        versions_job->addNetAction(check.download);
        checks.push_back(check);
    }

    setStatus(tr("Getting API responses from CurseForge..."));
    if (!checks.empty() && !runJob(versions_job)) {
        emitAborted();
        return;
    }

    setStatus(tr("Parsing the API responses from CurseForge..."));

    struct Update {
        Mod* mod;
        ModPlatform::IndexedVersion version;
        QString old_version;
        std::shared_ptr<QByteArray> changelog;
        std::shared_ptr<QByteArray> file_info;
        Net::Download::Ptr changelog_download;
        Net::Download::Ptr file_info_download;
    };
    std::vector<Update> updates;

    NetJob::Ptr details_job = new NetJob("Flame::CheckUpdateDetails", APPLICATION->network());
    for (auto& check : checks) {
        auto* mod = check.mod;
        auto latest_ver = FlameAPI::parseLatestVersion(succeededData(check.download, check.response));

        if (!latest_ver.addonId.isValid()) {
            emit checkFailed(mod, tr("No valid version found for this mod. It's probably unavailable for the current game "
                                     "version / mod loader."));
            continue;
        }

        if (latest_ver.downloadUrl.isEmpty() && latest_ver.fileId != mod->metadata()->file_id) {
            auto pack = getProjectInfo(latest_ver);
            if (m_was_aborted) {
                emitAborted();
                return;
            }
            auto recover_url = QString("%1/download/%2").arg(pack.websiteUrl, latest_ver.fileId.toString());
            emit checkFailed(mod, tr("Mod has a new update available, but is not downloadable using CurseForge."), recover_url);

            continue;
        }

        if (!latest_ver.hash.isEmpty() && (mod->metadata()->hash != latest_ver.hash || mod->status() == ModStatus::NotInstalled)) {
            Update update{ mod, latest_ver, mod->version(), std::make_shared<QByteArray>(), std::make_shared<QByteArray>(), nullptr, nullptr };
            update.changelog_download = Net::Download::makeByteArray(
                api.getModFileChangelogURL(latest_ver.addonId.toInt(), latest_ver.fileId.toInt()), update.changelog.get());
            details_job->addNetAction(update.changelog_download);
            if (update.old_version.isEmpty() && mod->status() != ModStatus::NotInstalled) {
                update.file_info_download = Net::Download::makeByteArray(
                    getFileInfoURL(latest_ver.addonId.toInt(), mod->metadata()->file_id.toInt()), update.file_info.get());
                details_job->addNetAction(update.file_info_download);
            }
            updates.push_back(update);
        }
    }

    setStatus(tr("Getting changelogs from CurseForge..."));
    if (!updates.empty() && !runJob(details_job)) {
        emitAborted();
        return;
    }

    for (auto& update : updates) {
        auto* mod = update.mod;

        // Fake pack with the necessary info to pass to the download task :)
        ModPlatform::IndexedPack pack;
        pack.name = mod->name();
        pack.slug = mod->metadata()->slug;
        pack.addonId = mod->metadata()->project_id;
        pack.websiteUrl = mod->homeurl();
        for (auto& author : mod->authors())
            pack.authors.append({ author, "" });
        pack.description = mod->description();
        pack.provider = ModPlatform::Provider::FLAME;

        auto file_info = succeededData(update.file_info_download, update.file_info);
        if (!file_info.isEmpty())
            update.old_version = parseFileInfo(file_info).version;

        auto changelog = succeededData(update.changelog_download, update.changelog);
        auto download_task = new ModDownloadTask(pack, update.version, m_mods_folder);
        m_updatable.emplace_back(pack.name, mod->metadata()->hash, update.old_version, update.version.version,
                                 changelog.isEmpty() ? QString() : FlameAPI::parseModFileChangelog(changelog),
                                 ModPlatform::Provider::FLAME, download_task);
    }

    emitSucceeded();
}
