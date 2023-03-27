// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 flowln <flowlnlnln@gmail.com>
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2020-2021 Jamie Mansfield <jmansfield@cadixdev.org>
 *      Copyright 2020-2021 Petr Mrazek <peterix@gmail.com>
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "FTBPackInstallTask.h"

#include "FileSystem.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "modplatform/flame/PackManifest.h"
#include "net/ChecksumValidator.h"
#include "settings/INISettingsObject.h"

#include "Application.h"
#include "BuildConfig.h"
#include "ui/dialogs/BlockedModsDialog.h"

namespace ModpacksCH {

PackInstallTask::PackInstallTask(Modpack pack, QString version, QWidget* parent)
    : m_pack(std::move(pack)), m_version_name(std::move(version)), m_parent(parent)
{}

bool PackInstallTask::abort()
{
    bool aborted = true;

    if (m_net_job)
        aborted &= m_net_job->abort();
    if (m_mod_id_resolver_task)
        aborted &= m_mod_id_resolver_task->abort();

    if (aborted)
        emitAborted();

    return aborted;
}

void PackInstallTask::executeTask()
{
    setStatus(tr("Getting the manifest..."));

    // Find pack version
    auto version_it = std::find_if(m_pack.versions.constBegin(), m_pack.versions.constEnd(),
                                   [this](ModpacksCH::VersionInfo const& a) { return a.name == m_version_name; });

    if (version_it == m_pack.versions.constEnd()) {
        emitFailed(tr("Failed to find pack version %1").arg(m_version_name));
        return;
    }

    auto version = *version_it;

    auto* netJob = new NetJob("ModpacksCH::VersionFetch", APPLICATION->network());

    auto searchUrl = QString(BuildConfig.MODPACKSCH_API_BASE_URL + "public/modpack/%1/%2").arg(m_pack.id).arg(version.id);
    netJob->addNetAction(Net::Download::makeByteArray(QUrl(searchUrl), &m_response));

    QObject::connect(netJob, &NetJob::succeeded, this, &PackInstallTask::onManifestDownloadSucceeded);
    QObject::connect(netJob, &NetJob::failed, this, &PackInstallTask::onManifestDownloadFailed);
    QObject::connect(netJob, &NetJob::progress, this, &PackInstallTask::setProgress);

    m_net_job = netJob;

    netJob->start();
}

void PackInstallTask::onManifestDownloadSucceeded()
{
    m_net_job.reset();

    nlohmann::json obj;
    try {
        obj = nlohmann::json::parse(m_response.constData(), m_response.constData() + m_response.size());
    } catch (const nlohmann::json::parse_error& e) {
        qWarning() << "Error while parsing JSON response from ModpacksCH at " << e.byte << " reason: " << e.what();
        qWarning() << m_response;
        return;
    }

    ModpacksCH::Version version;
    try {
        ModpacksCH::loadVersion(version, obj);
    } catch (const nlohmann::json::exception& e) {
        emitFailed(tr("Could not understand pack manifest:\n") + e.what());
        return;
    }

    m_version = version;

    resolveMods();
}

void PackInstallTask::resolveMods()
{
    setStatus(tr("Resolving mods..."));
    setProgress(0, 100);

    m_file_id_map.clear();

    Flame::Manifest manifest;
    int index = 0;

    for (auto const& file : m_version.files) {
        if (!file.serverOnly && file.url.isEmpty()) {
            if (file.curseforge.file_id <= 0) {
                emitFailed(tr("Invalid manifest: There's no information available to download the file '%1'!").arg(file.name));
                return;
            }

            Flame::File flame_file;
            flame_file.projectId = file.curseforge.project_id;
            flame_file.fileId = file.curseforge.file_id;
            flame_file.hash = file.sha1;

            manifest.files.insert(flame_file.fileId, flame_file);
            m_file_id_map.append(flame_file.fileId);
        } else {
            m_file_id_map.append(-1);
        }

        index++;
    }

    m_mod_id_resolver_task = new Flame::FileResolvingTask(APPLICATION->network(), manifest);

    connect(m_mod_id_resolver_task.get(), &Flame::FileResolvingTask::succeeded, this, &PackInstallTask::onResolveModsSucceeded);
    connect(m_mod_id_resolver_task.get(), &Flame::FileResolvingTask::failed, this, &PackInstallTask::onResolveModsFailed);
    connect(m_mod_id_resolver_task.get(), &Flame::FileResolvingTask::progress, this, &PackInstallTask::setProgress);

    m_mod_id_resolver_task->start();
}

void PackInstallTask::onResolveModsSucceeded()
{
    m_abortable = false;

    QString text;
    QList<QUrl> urls;
    auto anyBlocked = false;

    Flame::Manifest results = m_mod_id_resolver_task->getResults();
    for (int index = 0; index < m_file_id_map.size(); index++) {
        auto const file_id = m_file_id_map.at(index);
        if (file_id < 0)
            continue;

        Flame::File results_file = results.files[file_id];
        VersionFile& local_file = m_version.files[index];

        // First check for blocked mods
        if (!results_file.resolved || results_file.url.isEmpty()) {
            QString type(local_file.type);

            type[0] = type[0].toUpper();
            text += QString("%1: %2 - <a href='%3'>%3</a><br/>").arg(type, local_file.name, results_file.websiteUrl);
            urls.append(QUrl(results_file.websiteUrl));
            anyBlocked = true;
        } else {
            local_file.url = results_file.url.toString();
        }
    }

    m_mod_id_resolver_task.reset();

    if (anyBlocked) {
        qDebug() << "Blocked files found, displaying file list";

        auto message_dialog = new BlockedModsDialog(m_parent, tr("Blocked files found"),
                                                   tr("The following files are not available for download in third party launchers.<br/>"
                                                      "You will need to manually download them and add them to the instance."),
                                                   text,
                                                   urls);

        if (message_dialog->exec() == QDialog::Accepted)
            downloadPack();
        else
            abort();
    } else {
        downloadPack();
    }
}

void PackInstallTask::downloadPack()
{
    setStatus(tr("Downloading mods..."));

    auto* jobPtr = new NetJob(tr("Mod download"), APPLICATION->network());
    for (auto const& file : m_version.files) {
        if (file.serverOnly || file.url.isEmpty())
            continue;

        QFileInfo file_info(file.name);
        auto cacheName = file_info.completeBaseName() + "-" + file.sha1 + "." + file_info.suffix();

        auto entry = APPLICATION->metacache()->resolveEntry("ModpacksCHPacks", cacheName);
        entry->setStale(true);

        auto relpath = FS::PathCombine("minecraft", file.path, file.name);
        auto path = FS::PathCombine(m_stagingPath, relpath);

        if (m_files_to_copy.contains(path)) {
            qWarning() << "Ignoring" << file.url << "as a file of that path is already downloading.";
            continue;
        }

        qDebug() << "Will download" << file.url << "to" << path;
        m_files_to_copy[path] = entry->getFullPath();

        auto dl = Net::Download::makeCached(file.url, entry);
        if (!file.sha1.isEmpty()) {
            auto rawSha1 = QByteArray::fromHex(file.sha1.toLatin1());
            dl->addValidator(new Net::ChecksumValidator(QCryptographicHash::Sha1, rawSha1));
        }

        jobPtr->addNetAction(dl);
    }

    connect(jobPtr, &NetJob::succeeded, this, &PackInstallTask::onModDownloadSucceeded);
    connect(jobPtr, &NetJob::failed, this, &PackInstallTask::onModDownloadFailed);
    connect(jobPtr, &NetJob::progress, this, &PackInstallTask::setProgress);

    m_net_job = jobPtr;
    jobPtr->start();

    m_abortable = true;
}

void PackInstallTask::onModDownloadSucceeded()
{
    m_net_job.reset();
    install();
}

void PackInstallTask::install()
{
    setStatus(tr("Copying modpack files..."));
    setProgress(0, m_files_to_copy.size());
    QCoreApplication::processEvents();

    m_abortable = false;

    int i = 0;
    for (auto iter = m_files_to_copy.constBegin(); iter != m_files_to_copy.constEnd(); iter++) {
        auto& to = iter.key();
        auto& from = iter.value();
        FS::copy fileCopyOperation(from, to);
        if (!fileCopyOperation()) {
            qWarning() << "Failed to copy" << from << "to" << to;
            emitFailed(tr("Failed to copy files"));
            return;
        }

        setProgress(i++, m_files_to_copy.size());
        QCoreApplication::processEvents();
    }

    setStatus(tr("Installing modpack..."));
    QCoreApplication::processEvents();

    auto instanceConfigPath = FS::PathCombine(m_stagingPath, "instance.cfg");
    auto instanceSettings = std::make_shared<INISettingsObject>(instanceConfigPath);
    instanceSettings->suspendSave();

    MinecraftInstance instance(m_globalSettings, instanceSettings, m_stagingPath);
    auto components = instance.getPackProfile();
    components->buildingFromScratch();

    for (auto target : m_version.targets) {
        if (target.type == "game" && target.name == "minecraft") {
            components->setComponentVersion("net.minecraft", target.version, true);
            break;
        }
    }

    for (auto target : m_version.targets) {
        if (target.type != "modloader")
            continue;

        if (target.name == "forge") {
            components->setComponentVersion("net.minecraftforge", target.version);
        } else if (target.name == "fabric") {
            components->setComponentVersion("net.fabricmc.fabric-loader", target.version);
        }
    }

    // install any jar mods
    QDir jarModsDir(FS::PathCombine(m_stagingPath, "minecraft", "jarmods"));
    if (jarModsDir.exists()) {
        QStringList jarMods;

        for (const auto& info : jarModsDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files)) {
            jarMods.push_back(info.absoluteFilePath());
        }

        components->installJarMods(jarMods);
    }

    components->saveNow();

    instance.setName(name());
    instance.setIconKey(m_instIcon);
    instance.setManagedPack("modpacksch", QString::number(m_pack.id), m_pack.name, QString::number(m_version.id), m_version.name);
    instanceSettings->resumeSave();

    emitSucceeded();
}

void PackInstallTask::onManifestDownloadFailed(QString reason)
{
    m_net_job.reset();
    emitFailed(reason);
}
void PackInstallTask::onResolveModsFailed(QString reason)
{
    m_net_job.reset();
    emitFailed(reason);
}
void PackInstallTask::onModDownloadFailed(QString reason)
{
    m_net_job.reset();
    emitFailed(reason);
}

}  // namespace ModpacksCH
