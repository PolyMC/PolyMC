#include "ModrinthInstanceCreationTask.h"

#include "Application.h"
#include "FileSystem.h"
#include "InstanceList.h"

#include "minecraft/PackProfile.h"

#include "modplatform/helpers/OverrideUtils.h"

#include "net/ChecksumValidator.h"

#include "settings/INISettingsObject.h"

#include "ui/dialogs/CustomMessageBox.h"

#include <QAbstractButton>
#include <fstream>

bool ModrinthCreationTask::abort()
{
    if (!canAbort())
        return false;

    m_abort = true;
    if (m_files_job)
        m_files_job->abort();
    return Task::abort();
}

bool ModrinthCreationTask::updateInstance()
{
    auto instance_list = APPLICATION->instances();

    // FIXME: How to handle situations when there's more than one install already for a given modpack?
    auto inst = instance_list->getInstanceByManagedName(originalName());

    if (!inst) {
        inst = instance_list->getInstanceById(originalName());

        if (!inst)
            return false;
    }

    QString index_path = FS::PathCombine(m_stagingPath, "modrinth.index.json");
    if (!parseManifest(index_path, m_files, true, false))
        return false;

    auto version_name = inst->getManagedPackVersionName();
    auto version_str = !version_name.isEmpty() ? tr(" (version %1)").arg(version_name) : "";

    auto info = CustomMessageBox::selectable(
        m_parent, tr("Similar modpack was found!"),
        tr("One or more of your instances are from this same modpack%1. Do you want to create a "
           "separate instance, or update the existing one?\n\nNOTE: Make sure you made a backup of your important instance data before "
           "updating, as worlds can be corrupted and some configuration may be lost (due to pack overrides).")
            .arg(version_str),
        QMessageBox::Information, QMessageBox::Ok | QMessageBox::Reset | QMessageBox::Abort);
    info->setButtonText(QMessageBox::Ok, tr("Create new instance"));
    info->setButtonText(QMessageBox::Abort, tr("Update existing instance"));
    info->setButtonText(QMessageBox::Reset, tr("Cancel"));

    info->exec();

    if (info->clickedButton() == info->button(QMessageBox::Ok))
        return false;

    if (info->clickedButton() == info->button(QMessageBox::Reset)) {
        m_abort = true;
        return false;
    }

    // Remove repeated files, we don't need to download them!
    QDir old_inst_dir(inst->instanceRoot());

    QString old_index_folder(FS::PathCombine(old_inst_dir.absolutePath(), "mrpack"));

    QString old_index_path(FS::PathCombine(old_index_folder, "modrinth.index.json"));
    QFileInfo old_index_file(old_index_path);
    if (old_index_file.exists()) {
        std::vector<Modrinth::File> old_files;
        parseManifest(old_index_path, old_files, false, false);

        // Let's remove all duplicated, identical resources!
        auto files_iterator = m_files.begin();
    begin:
        while (files_iterator != m_files.end()) {
            auto const& file = *files_iterator;

            auto old_files_iterator = old_files.begin();
            while (old_files_iterator != old_files.end()) {
                auto const& old_file = *old_files_iterator;

                if (old_file.hash == file.hash) {
                    qDebug() << "Removed file at" << file.path << "from list of downloads";
                    files_iterator = m_files.erase(files_iterator);
                    old_files_iterator = old_files.erase(old_files_iterator);
                    goto begin;  // Sorry :c
                }

                old_files_iterator++;
            }

            files_iterator++;
        }

        QDir old_minecraft_dir(inst->gameRoot());

        // Some files were removed from the old version, and some will be downloaded in an updated version,
        // so we're fine removing them!
        if (!old_files.empty()) {
            for (auto const& file : old_files) {
                if (file.path.isEmpty())
                    continue;
                qDebug() << "Scheduling" << file.path << "for removal";
                m_files_to_remove.append(old_minecraft_dir.absoluteFilePath(file.path));
            }
        }

        // We will remove all the previous overrides, to prevent duplicate files!
        // TODO: Currently 'overrides' will always override the stuff on update. How do we preserve unchanged overrides?
        // FIXME: We may want to do something about disabled mods.
        auto old_overrides = Override::readOverrides("overrides", old_index_folder);
        for (const auto& entry : old_overrides) {
            if (entry.isEmpty())
                continue;
            qDebug() << "Scheduling" << entry << "for removal";
            m_files_to_remove.append(old_minecraft_dir.absoluteFilePath(entry));
        }

        auto old_client_overrides = Override::readOverrides("client-overrides", old_index_folder);
        for (const auto& entry : old_overrides) {
            if (entry.isEmpty())
                continue;
            qDebug() << "Scheduling" << entry << "for removal";
            m_files_to_remove.append(old_minecraft_dir.absoluteFilePath(entry));
        }
    } else {
        // We don't have an old index file, so we may duplicate stuff!
        auto dialog = CustomMessageBox::selectable(m_parent,
                tr("No index file."),
                tr("We couldn't find a suitable index file for the older version. This may cause some of the files to be duplicated. Do you want to continue?"),
                QMessageBox::Warning, QMessageBox::Ok | QMessageBox::Cancel);

        if (dialog->exec() == QDialog::DialogCode::Rejected) {
            m_abort = true;
            return false;
        }
    }


    setOverride(true);
    qDebug() << "Will override instance!";

    m_instance = inst;

    // We let it go through the createInstance() stage, just with a couple modifications for updating
    return false;
}

// https://docs.modrinth.com/docs/modpacks/format_definition/
bool ModrinthCreationTask::createInstance()
{
    QEventLoop loop;

    QString parent_folder(FS::PathCombine(m_stagingPath, "mrpack"));

    QString index_path = FS::PathCombine(m_stagingPath, "modrinth.index.json");
    if (m_files.empty() && !parseManifest(index_path, m_files, true, true))
        return false;

    // Keep index file in case we need it some other time (like when changing versions)
    QString new_index_place(FS::PathCombine(parent_folder, "modrinth.index.json"));
    FS::ensureFilePathExists(new_index_place);
    QFile::rename(index_path, new_index_place);

    auto mcPath = FS::PathCombine(m_stagingPath, ".minecraft");

    auto override_path = FS::PathCombine(m_stagingPath, "overrides");
    if (QFile::exists(override_path)) {
        // Create a list of overrides in "overrides.txt" inside mrpack/
        Override::createOverrides("overrides", parent_folder, override_path);

        // Apply the overrides
        if (!QFile::rename(override_path, mcPath)) {
            setError(tr("Could not rename the overrides folder:\n") + "overrides");
            return false;
        }
    }

    // Do client overrides
    auto client_override_path = FS::PathCombine(m_stagingPath, "client-overrides");
    if (QFile::exists(client_override_path)) {
        // Create a list of overrides in "client-overrides.txt" inside mrpack/
        Override::createOverrides("client-overrides", parent_folder, client_override_path);

        // Apply the overrides
        if (!FS::overrideFolder(mcPath, client_override_path)) {
            setError(tr("Could not rename the client overrides folder:\n") + "client overrides");
            return false;
        }
    }

    QString configPath = FS::PathCombine(m_stagingPath, "instance.cfg");
    auto instanceSettings = std::make_shared<INISettingsObject>(configPath);
    MinecraftInstance instance(m_globalSettings, instanceSettings, m_stagingPath);

    auto components = instance.getPackProfile();
    components->buildingFromScratch();
    components->setComponentVersion("net.minecraft", minecraftVersion, true);

    if (!fabricVersion.isEmpty())
        components->setComponentVersion("net.fabricmc.fabric-loader", fabricVersion);
    if (!quiltVersion.isEmpty())
        components->setComponentVersion("org.quiltmc.quilt-loader", quiltVersion);
    if (!forgeVersion.isEmpty())
        components->setComponentVersion("net.minecraftforge", forgeVersion);

    if (m_instIcon != "default") {
        instance.setIconKey(m_instIcon);
    } else {
        instance.setIconKey("modrinth");
    }

    instance.setManagedPack("modrinth", getManagedPackID(), m_managed_name, m_managed_version_id, version());
    instance.setName(name());
    instance.saveNow();

    m_files_job = new NetJob(tr("Mod download"), APPLICATION->network());

    for (auto file : m_files) {
        auto path = FS::PathCombine(m_stagingPath, ".minecraft", file.path);
        qDebug() << "Will try to download" << file.downloads.front() << "to" << path;
        auto dl = Net::Download::makeFile(file.downloads.dequeue(), path);
        dl->addValidator(new Net::ChecksumValidator(file.hashAlgorithm, file.hash));
        m_files_job->addNetAction(dl);

        if (!file.downloads.empty()) {
            // FIXME: This really needs to be put into a ConcurrentTask of
            // MultipleOptionsTask's , once those exist :)
            auto param = dl.toWeakRef();
            connect(dl.get(), &NetAction::failed, [this, &file, path, param] {
                auto ndl = Net::Download::makeFile(file.downloads.dequeue(), path);
                ndl->addValidator(new Net::ChecksumValidator(file.hashAlgorithm, file.hash));
                m_files_job->addNetAction(ndl);
                if (auto shared = param.lock()) shared->succeeded();
            });
        }
    }

    bool ended_well = false;

    connect(m_files_job.get(), &NetJob::succeeded, this, [&]() { ended_well = true; });
    connect(m_files_job.get(), &NetJob::failed, [&](const QString& reason) {
        ended_well = false;
        setError(reason);
    });
    connect(m_files_job.get(), &NetJob::finished, &loop, &QEventLoop::quit);
    connect(m_files_job.get(), &NetJob::progress, [&](qint64 current, qint64 total) { setProgress(current, total); });

    setStatus(tr("Downloading mods..."));
    m_files_job->start();

    loop.exec();

    // Update information of the already installed instance, if any.
    if (m_instance && ended_well) {
        setAbortable(false);
        auto inst = m_instance.value();

        // Only change the name if it didn't use a custom name, so that the previous custom name
        // is preserved, but if we're using the original one, we update the version string.
        // NOTE: This needs to come before the copyManagedPack call!
        if (inst->name().contains(inst->getManagedPackVersionName())) {
            if (askForChangingInstanceName(m_parent, inst->name(), instance.name()) == InstanceNameChange::ShouldChange)
                inst->setName(instance.name());
        }

        inst->copyManagedPack(instance);
    }

    return ended_well;
}

bool ModrinthCreationTask::parseManifest(const QString& index_path, std::vector<Modrinth::File>& files, bool set_managed_info, bool show_optional_dialog)
{
    try {
        nlohmann::json obj = nlohmann::json::parse(std::ifstream(index_path.toStdString()));

        int formatVersion = obj["formatVersion"];
        if (formatVersion == 1) {
            auto game = obj["game"].get<std::string>();
            if (obj["game"].get<std::string>() != "minecraft") {
                throw std::runtime_error("Unsupported game: " + game);
            }

            if (set_managed_info) {
                m_managed_version_id = obj.value("versionId", "").c_str();
                m_managed_name = obj.value("name", "").c_str();
            }

            bool had_optional = false;
            for (const auto& modInfo : obj["files"]) {
                Modrinth::File file;
                file.path = modInfo["path"].get<std::string>().c_str();

                if (QDir::isAbsolutePath(file.path) || QDir::cleanPath(file.path).startsWith("..")) {
                    qDebug() << "Skipped file that tries to place itself in an absolute location or in a parent directory.";
                    continue;
                }

                auto env = modInfo.value("env", nlohmann::json::object());
                // 'env' field is optional
                if (!env.empty()) {
                    QString support = env.value("client", "unsupported").c_str();
                    if (support == "unsupported") {
                        continue;
                    } else if (support == "optional") {
                        // TODO: Make a review dialog for choosing which ones the user wants!
                        if (!had_optional && show_optional_dialog) {
                            had_optional = true;
                            auto info = CustomMessageBox::selectable(
                                m_parent, tr("Optional mod detected!"),
                                tr("One or more mods from this modpack are optional. They will be downloaded, but disabled by default!"),
                                QMessageBox::Information);
                            info->exec();
                        }

                        if (file.path.endsWith(".jar"))
                            file.path += ".disabled";
                    }
                }

                auto hashes = modInfo.value("hashes", nlohmann::json::object());
                QString hash;
                QCryptographicHash::Algorithm hashAlgorithm;
                hash = hashes.value("sha1", "").c_str();
                hashAlgorithm = QCryptographicHash::Sha1;
                if (hash.isEmpty()) {
                    hash = hashes.value("sha512", "").c_str();
                    hashAlgorithm = QCryptographicHash::Sha512;
                    if (hash.isEmpty()) {
                        hash = hashes.value("sha256", "").c_str();
                        hashAlgorithm = QCryptographicHash::Sha256;
                        if (hash.isEmpty()) {
                            throw std::runtime_error("No hash found for file " + file.path.toStdString());
                        }
                    }
                }
                file.hash = QByteArray::fromHex(hash.toLatin1());
                file.hashAlgorithm = hashAlgorithm;

                // Do not use requireUrl, which uses StrictMode, instead use QUrl's default TolerantMode
                // (as Modrinth seems to incorrectly handle spaces)

                auto download_arr = modInfo["downloads"];
                for (const auto& download : download_arr) {
                    //qWarning() << download.toString();
                    qWarning() << download.get<std::string>().c_str();
                    //bool is_last = download.toString() == download_arr.last().toString();
                    bool is_last = download.get<std::string>().c_str() == download_arr.back().get<std::string>().c_str();

                    auto download_url = QUrl(download.get<std::string>().c_str());

                    if (!download_url.isValid()) {
                        qDebug()
                            << QString("Download URL (%1) for %2 is not a correctly formatted URL").arg(download_url.toString(), file.path);
                        if (is_last && file.downloads.isEmpty())
                            throw std::runtime_error("Download URL for " + file.path.toStdString() + " is not a correctly formatted URL");
                    } else {
                        file.downloads.push_back(download_url);
                    }
                }

                files.push_back(file);
            }

            auto dependencies = obj.value("dependencies", nlohmann::json::object());
            for (auto it = dependencies.begin(), end = dependencies.end(); it != end; ++it) {
                const std::string& name = it.key();
                if (name == "minecraft") {
                    minecraftVersion = it.value().get<std::string>().c_str();
                } else if (name == "fabric-loader") {
                    fabricVersion = it.value().get<std::string>().c_str();
                } else if (name == "quilt-loader") {
                    quiltVersion = it.value().get<std::string>().c_str();
                } else if (name == "forge") {
                    forgeVersion = it.value().get<std::string>().c_str();
                } else {
                    throw std::runtime_error("Unknown dependency type: " + name);
                }
            }

        } else {
            throw std::runtime_error("Unknown format version: " + std::to_string(formatVersion));
        }

    } catch (const std::exception& e){
        setError(tr("Could not understand pack index:\n") + e.what());
        return false;
    }

    return true;
}

QString ModrinthCreationTask::getManagedPackID() const
{
    if (!m_source_url.isEmpty()) {
        QRegularExpression regex(R"(data\/(.*)\/versions)");
        return regex.match(m_source_url).captured(1);
    }

    return {};
}
