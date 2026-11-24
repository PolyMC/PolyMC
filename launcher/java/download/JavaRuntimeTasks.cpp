// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
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
 */

#include "JavaRuntimeTasks.h"

#include <QDebug>
#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QThread>
#include <QUrlQuery>
#include <QtConcurrentRun>

#include <algorithm>
#include <set>

#include "Application.h"
#include "ArchiveExtractor.h"
#include "FileSystem.h"
#include "net/ChecksumValidator.h"

namespace JavaRuntime
{

namespace
{

const QString MOJANG_INDEX =
    "https://launchermeta.mojang.com/v1/products/java-runtime/"
    "2ec0cc96c44e5a76b9c8b7c39df7210883d12871/all.json";
const QString ADOPTIUM_API = "https://api.adoptium.net/v3";
const QString AZUL_API = "https://api.azul.com/metadata/v1/zulu/packages/";
// IBM no longer runs a Semeru metadata API, the GitHub releases are the source
// of truth.
const QString SEMERU_RELEASES =
    "https://api.github.com/repos/ibmruntimes/semeru%1-binaries/releases";
const QList<int> SEMERU_MAJORS = {25, 21, 17, 11, 8};

QString mojangPlatform()
{
    auto arch = hostArch();
#if defined(Q_OS_WIN)
    if (arch == "x64")
        return "windows-x64";
    if (arch == "x86")
        return "windows-x86";
    if (arch == "aarch64")
        return "windows-arm64";
#elif defined(Q_OS_MACOS)
    if (arch == "aarch64")
        return "mac-os-arm64";
    if (arch == "x64")
        return "mac-os";
#elif defined(Q_OS_LINUX)
    if (arch == "x64")
    {
        return "linux";
    }
    if (arch == "x86")
    {
        return "linux-i386";
    }
#endif
    return {};
}

QString vendorOs()
{
#if defined(Q_OS_WIN)
    return "windows";
#elif defined(Q_OS_MACOS)
    return "mac";
#elif defined(Q_OS_LINUX)
    return "linux";
#else
    return {};
#endif
}

QString azulOs()
{
#if defined(Q_OS_WIN)
    return "windows";
#elif defined(Q_OS_MACOS)
    return "macos";
#elif defined(Q_OS_LINUX)
    return "linux-glibc";
#else
    return {};
#endif
}

QString archiveType()
{
#ifdef Q_OS_WIN
    return "zip";
#else
    return "tar.gz";
#endif
}

bool platformSupported()
{
    return !hostArch().isEmpty() && !vendorOs().isEmpty();
}

QUrl azulUrl(std::optional<int> major)
{
    QUrl url(AZUL_API);
    QUrlQuery query;
    if (major)
    {
        query.addQueryItem("java_version", QString::number(*major));
    }
    query.addQueryItem("os", azulOs());
    query.addQueryItem("arch", hostArch());
    query.addQueryItem("archive_type", archiveType());
    query.addQueryItem("javafx_bundled", "false");
    query.addQueryItem("crac_supported", "false");
    query.addQueryItem("release_status", "ga");
    query.addQueryItem("availability_types", "CA");
    if (major)
    {
        query.addQueryItem("page_size", "40");
        query.addQueryItem("include_fields",
                           "java_package_type,build_date,sha256_hash");
    }
    else
    {
        query.addQueryItem("latest", "true");
        query.addQueryItem("page_size", "1000");
    }
    url.setQuery(query);
    return url;
}

QUrl releaseListUrl(Provider provider, int major)
{
    switch (provider)
    {
        case Provider::Mojang:
            return MOJANG_INDEX;
        case Provider::Adoptium:
        {
            QUrl url(QString("%1/assets/feature_releases/%2/ga")
                         .arg(ADOPTIUM_API)
                         .arg(major));
            QUrlQuery query;
            query.addQueryItem("os", vendorOs());
            query.addQueryItem("architecture", hostArch());
            query.addQueryItem("jvm_impl", "hotspot");
            query.addQueryItem("page_size", "20");
            query.addQueryItem("sort_order", "DESC");
            url.setQuery(query);
            return url;
        }
        case Provider::Azul:
            return azulUrl(major);
        case Provider::Semeru:
            return (SEMERU_RELEASES + "?per_page=20").arg(major);
    }
    return {};
}

// Mojang's index also lists things that are not Java runtimes, like
// minecraft-java-exe on Windows
bool isMojangRuntimeComponent(const QString& component)
{
    return component.startsWith("java-runtime-") ||
           component.startsWith("jre-");
}

// "8u202", "1.8.0_51", "17.0.15" -> major version
int majorFromVersionName(const QString& name)
{
    auto parts = name.split(QRegularExpression("[^0-9]+"), Qt::SkipEmptyParts);
    if (parts.isEmpty())
    {
        return 0;
    }
    int first = parts[0].toInt();
    if (first == 1 && parts.size() > 1)
    {
        return parts[1].toInt();
    }
    return first;
}

QJsonDocument parseJson(const QByteArray& data)
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
    {
        qWarning() << "Could not parse Java metadata:" << error.errorString();
    }
    return doc;
}

QDateTime parseDate(const QString& text)
{
    auto date = QDateTime::fromString(text, Qt::ISODate);
    if (!date.isValid())
    {
        date = QDateTime::fromString(text, Qt::ISODateWithMs);
    }
    return date;
}

// Path of a file inside root, or an empty string if the name would escape it.
QString pathInside(const QDir& root, const QString& name)
{
    if (name.isEmpty() || QDir::isAbsolutePath(name) || name.contains('\\'))
    {
        return {};
    }
    auto rootPath = QDir::cleanPath(root.absolutePath());
    auto path = QDir::cleanPath(root.absoluteFilePath(name));
    if (!path.startsWith(rootPath + '/'))
    {
        return {};
    }
    return path;
}

QString findJavaBinary(const QString& directory)
{
    QString best;
    QDirIterator it(directory, {javaBinaryName()}, QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        auto path = it.next();
        if (QFileInfo(path).dir().dirName() != "bin")
        {
            continue;
        }
        // a JDK 8 has both bin/java and jre/bin/java, the outer one is the
        // right one
        if (best.isEmpty() || path.size() < best.size())
        {
            best = path;
        }
    }
    return best;
}

// Virus scanners on Windows briefly lock freshly written files, which makes
// renaming their folder fail. Retry for a moment, then copy instead.
bool moveDirectory(const QString& source, const QString& target)
{
    for (int attempt = 0; attempt < 10; attempt++)
    {
        if (QDir().rename(source, target))
        {
            return true;
        }
        QThread::msleep(200);
    }
    qWarning() << "Could not rename" << source << "to" << target
               << ", copying instead";
    if (!FS::copy(source, target)())
    {
        FS::deletePath(target);
        return false;
    }
    FS::deletePath(source);
    return true;
}

void makeExecutable(const QString& path)
{
    QFile file(path);
    file.setPermissions(file.permissions() | QFileDevice::ExeOwner |
                        QFileDevice::ExeUser | QFileDevice::ExeGroup |
                        QFileDevice::ExeOther);
}

QList<Release> parseMojang(const QJsonDocument& doc, int major)
{
    QList<Release> result;
    auto platform = doc.object().value(mojangPlatform()).toObject();
    for (auto it = platform.begin(); it != platform.end(); ++it)
    {
        auto entries = it.value().toArray();
        if (entries.isEmpty() || !isMojangRuntimeComponent(it.key()))
        {
            continue;
        }
        auto entry = entries[0].toObject();
        auto version = entry.value("version").toObject();
        if (majorFromVersionName(version.value("name").toString()) != major)
        {
            continue;
        }
        auto manifest = entry.value("manifest").toObject();
        Release release;
        release.provider = Provider::Mojang;
        release.major = major;
        release.version = version.value("name").toString();
        release.name = it.key();
        release.released = parseDate(version.value("released").toString());
        release.type = "jre";
        release.url = manifest.value("url").toString();
        release.sha1 =
            QByteArray::fromHex(manifest.value("sha1").toString().toLatin1());
        release.mojangComponent = it.key();
        if (!release.url.isEmpty())
        {
            result.append(release);
        }
    }
    return result;
}

QList<Release> parseAdoptium(const QJsonDocument& doc, int major)
{
    QList<Release> result;
    for (auto value : doc.array())
    {
        auto obj = value.toObject();
        for (auto binaryValue : obj.value("binaries").toArray())
        {
            auto binary = binaryValue.toObject();
            auto type = binary.value("image_type").toString();
            if (type != "jre" && type != "jdk")
            {
                continue;
            }
            auto package = binary.value("package").toObject();
            Release release;
            release.provider = Provider::Adoptium;
            release.major = major;
            release.version = obj.value("version_data")
                                  .toObject()
                                  .value("openjdk_version")
                                  .toString();
            release.name = obj.value("release_name").toString();
            release.released = parseDate(binary.value("updated_at").toString());
            if (!release.released.isValid())
            {
                release.released = parseDate(obj.value("timestamp").toString());
            }
            release.type = type;
            release.url = package.value("link").toString();
            release.sha256 = QByteArray::fromHex(
                package.value("checksum").toString().toLatin1());
            if (!release.url.isEmpty())
            {
                result.append(release);
            }
        }
    }
    return result;
}

QList<Release> parseAzul(const QJsonDocument& doc, int major)
{
    QList<Release> result;
    auto suffix = "." + archiveType();
    for (auto value : doc.array())
    {
        auto obj = value.toObject();
        auto type = obj.value("java_package_type").toString();
        if (type != "jre" && type != "jdk")
        {
            continue;
        }
        QStringList version;
        for (auto part : obj.value("java_version").toArray())
        {
            version.append(QString::number(part.toInt()));
        }
        Release release;
        release.provider = Provider::Azul;
        release.major = major;
        release.version = version.join('.');
        release.name = obj.value("name").toString();
        if (release.name.endsWith(suffix))
        {
            release.name.chop(suffix.size());
        }
        release.released = parseDate(obj.value("build_date").toString());
        release.type = type;
        release.url = obj.value("download_url").toString();
        release.sha256 =
            QByteArray::fromHex(obj.value("sha256_hash").toString().toLatin1());
        if (!release.url.isEmpty())
        {
            result.append(release);
        }
    }
    return result;
}

QList<Release> parseSemeru(const QJsonDocument& doc, int major)
{
    QList<Release> result;
    auto suffix = "." + archiveType();
    for (auto value : doc.array())
    {
        auto obj = value.toObject();
        if (obj.value("draft").toBool() || obj.value("prerelease").toBool())
        {
            continue;
        }
        for (auto type : {QString("jre"), QString("jdk")})
        {
            auto prefix = QString("ibm-semeru-open-%1_%2_%3_")
                              .arg(type, hostArch(), vendorOs());
            for (auto assetValue : obj.value("assets").toArray())
            {
                auto asset = assetValue.toObject();
                auto name = asset.value("name").toString();
                if (!name.startsWith(prefix) || !name.endsWith(suffix))
                {
                    continue;
                }
                Release release;
                release.provider = Provider::Semeru;
                release.major = major;
                release.version = name.mid(
                    prefix.size(), name.size() - prefix.size() - suffix.size());
                release.name = obj.value("tag_name").toString();
                release.released =
                    parseDate(obj.value("published_at").toString());
                release.type = type;
                release.url = asset.value("browser_download_url").toString();
                auto digest = asset.value("digest").toString();
                if (digest.startsWith("sha256:"))
                {
                    release.sha256 =
                        QByteArray::fromHex(digest.mid(7).toLatin1());
                }
                result.append(release);
                break;
            }
        }
    }
    return result;
}

bool newerRelease(const Release& a, const Release& b)
{
    return a.released > b.released;
}

}  // namespace

bool isLts(int major)
{
    return major == 8 || major == 11 || (major >= 17 && (major - 17) % 4 == 0);
}

// ---------------------------------------------------------------------------------------------------------------------

VersionListTask::VersionListTask(Provider provider)
    : Task(nullptr), m_provider(provider)
{
    setAbortable(true);
}

bool VersionListTask::abort()
{
    if (m_job)
    {
        m_job->abort();
    }
    emitAborted();
    return true;
}

void VersionListTask::executeTask()
{
    if (!platformSupported())
    {
        emitFailed(tr("Downloading Java is not supported on this platform."));
        return;
    }

    QUrl url;
    switch (m_provider)
    {
        case Provider::Mojang:
            url = MOJANG_INDEX;
            break;
        case Provider::Adoptium:
            url = ADOPTIUM_API + "/info/available_releases";
            break;
        case Provider::Azul:
            url = azulUrl(std::nullopt);
            break;
        case Provider::Semeru:
            m_majors = SEMERU_MAJORS;
            emitSucceeded();
            return;
    }

    setStatus(
        tr("Fetching Java versions from %1...").arg(providerName(m_provider)));
    m_job.reset(new NetJob(tr("Java version list"), APPLICATION->network()));
    m_job->addNetAction(Net::Download::makeByteArray(url, m_data.get()));
    connect(m_job.get(), &Task::succeeded, this,
            &VersionListTask::jobSucceeded);
    connect(m_job.get(), &Task::failed, this, &VersionListTask::jobFailed);
    connect(m_job.get(), &Task::progress, this, &Task::setProgress);
    m_job->start();
}

void VersionListTask::jobSucceeded()
{
    m_job.reset();
    parse();
    emitSucceeded();
}

void VersionListTask::jobFailed(QString reason)
{
    m_job.reset();
    emitFailed(reason);
}

void VersionListTask::parse()
{
    auto doc = parseJson(*m_data);
    std::set<int> majors;

    switch (m_provider)
    {
        case Provider::Mojang:
        {
            auto platform = doc.object().value(mojangPlatform()).toObject();
            for (auto it = platform.begin(); it != platform.end(); ++it)
            {
                auto entries = it.value().toArray();
                if (!entries.isEmpty() && isMojangRuntimeComponent(it.key()))
                {
                    majors.insert(majorFromVersionName(entries[0]
                                                           .toObject()
                                                           .value("version")
                                                           .toObject()
                                                           .value("name")
                                                           .toString()));
                }
            }
            break;
        }
        case Provider::Adoptium:
            for (auto value :
                 doc.object().value("available_releases").toArray())
            {
                majors.insert(value.toInt());
            }
            break;
        case Provider::Azul:
            for (auto value : doc.array())
            {
                auto version = value.toObject().value("java_version").toArray();
                if (!version.isEmpty())
                {
                    majors.insert(version[0].toInt());
                }
            }
            break;
        case Provider::Semeru:
            break;
    }

    for (auto it = majors.rbegin(); it != majors.rend(); ++it)
    {
        if (*it >= 8)
        {
            m_majors.append(*it);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------

ReleaseListTask::ReleaseListTask(Provider provider, int major)
    : Task(nullptr), m_provider(provider), m_major(major)
{
    setAbortable(true);
}

bool ReleaseListTask::abort()
{
    if (m_job)
    {
        m_job->abort();
    }
    emitAborted();
    return true;
}

void ReleaseListTask::executeTask()
{
    if (!platformSupported())
    {
        emitFailed(tr("Downloading Java is not supported on this platform."));
        return;
    }

    setStatus(tr("Fetching Java %1 builds from %2...")
                  .arg(m_major)
                  .arg(providerName(m_provider)));
    m_job.reset(new NetJob(tr("Java release list"), APPLICATION->network()));
    m_job->addNetAction(Net::Download::makeByteArray(
        releaseListUrl(m_provider, m_major), m_data.get()));
    connect(m_job.get(), &Task::succeeded, this,
            &ReleaseListTask::jobSucceeded);
    connect(m_job.get(), &Task::failed, this, &ReleaseListTask::jobFailed);
    connect(m_job.get(), &Task::progress, this, &Task::setProgress);
    m_job->start();
}

void ReleaseListTask::jobSucceeded()
{
    m_job.reset();
    parse();
    emitSucceeded();
}

void ReleaseListTask::jobFailed(QString reason)
{
    m_job.reset();
    emitFailed(reason);
}

void ReleaseListTask::parse()
{
    auto doc = parseJson(*m_data);
    switch (m_provider)
    {
        case Provider::Mojang:
            m_releases = parseMojang(doc, m_major);
            break;
        case Provider::Adoptium:
            m_releases = parseAdoptium(doc, m_major);
            break;
        case Provider::Azul:
            m_releases = parseAzul(doc, m_major);
            break;
        case Provider::Semeru:
            m_releases = parseSemeru(doc, m_major);
            break;
    }
    std::stable_sort(m_releases.begin(), m_releases.end(), newerRelease);
}

std::optional<Release> ReleaseListTask::recommended() const
{
    // the list is sorted newest first
    for (auto type : {QString("jre"), QString("jdk")})
    {
        for (auto& release : m_releases)
        {
            if (release.type == type &&
                !release.mojangComponent.endsWith("-snapshot"))
            {
                return release;
            }
        }
    }
    return std::nullopt;
}

// ---------------------------------------------------------------------------------------------------------------------

InstallTask::InstallTask(Release release)
    : Task(nullptr), m_provider(release.provider), m_release(std::move(release))
{
    setAbortable(true);
    connect(&m_extractWatcher, &QFutureWatcher<QString>::finished, this,
            &InstallTask::extractFinished);
}

InstallTask::InstallTask(Provider provider, QList<int> majors)
    : Task(nullptr), m_provider(provider), m_majors(std::move(majors))
{
    setAbortable(true);
    connect(&m_extractWatcher, &QFutureWatcher<QString>::finished, this,
            &InstallTask::extractFinished);
}

bool InstallTask::abort()
{
    if (!canAbort())
    {
        return false;
    }
    m_aborted = true;
    if (m_listTask)
    {
        disconnect(m_listTask.get(), nullptr, this, nullptr);
        m_listTask->abort();
    }
    if (m_job)
    {
        m_job->abort();
    }
    emitAborted();
    return true;
}

void InstallTask::runJob(NetJob::Ptr job, void (InstallTask::*next)())
{
    m_job = job;
    m_afterJob = next;
    connect(job.get(), &Task::succeeded, this, &InstallTask::jobSucceeded);
    connect(job.get(), &Task::failed, this, &InstallTask::jobFailed);
    connect(job.get(), &Task::progress, this, &Task::setProgress);
    job->start();
}

void InstallTask::jobSucceeded()
{
    m_job.reset();
    if (!m_aborted)
    {
        (this->*m_afterJob)();
    }
}

void InstallTask::jobFailed(QString reason)
{
    m_job.reset();
    if (!m_aborted)
    {
        emitFailed(reason);
    }
}

void InstallTask::executeTask()
{
    if (!platformSupported())
    {
        emitFailed(tr("Downloading Java is not supported on this platform."));
        return;
    }
    if (!m_release.url.isEmpty())
    {
        download();
        return;
    }
    if (m_majors.isEmpty())
    {
        emitFailed(tr("No Java version was requested."));
        return;
    }
    resolveNext();
}

void InstallTask::resolveNext()
{
    if (m_majorIndex >= m_majors.size())
    {
        QStringList majors;
        for (int major : m_majors)
        {
            majors.append(QString::number(major));
        }
        auto message = tr("%1 does not provide Java %2 for this platform.")
                           .arg(providerName(m_provider), majors.join(", "));
        if (!m_lastError.isEmpty())
        {
            message += "\n" + tr("Last error: %1").arg(m_lastError);
        }
        emitFailed(message);
        return;
    }

    int major = m_majors[m_majorIndex];
    setStatus(tr("Looking for Java %1 from %2...")
                  .arg(major)
                  .arg(providerName(m_provider)));
    m_listTask.reset(new ReleaseListTask(m_provider, major));
    connect(m_listTask.get(), &Task::finished, this,
            &InstallTask::releaseListFinished);
    m_listTask->start();
}

void InstallTask::releaseListFinished()
{
    auto task = m_listTask;
    m_listTask.reset();
    if (m_aborted)
    {
        return;
    }
    auto release = task->wasSuccessful() ? task->recommended() : std::nullopt;
    if (!task->wasSuccessful())
    {
        m_lastError = task->failReason();
    }
    if (release)
    {
        m_release = *release;
        download();
        return;
    }
    // a missing version is not fatal, the next compatible major might be
    // available
    m_majorIndex++;
    resolveNext();
}

void InstallTask::download()
{
    if (!QDir().mkpath(runtimesRoot()))
    {
        emitFailed(tr("Could not create %1").arg(runtimesRoot()));
        return;
    }
    // inside the runtimes folder, so the final move is a cheap rename on the
    // same file system
    m_tempDir = std::make_unique<QTemporaryDir>(
        QDir(runtimesRoot()).absoluteFilePath(".download-XXXXXX"));
    if (!m_tempDir->isValid())
    {
        emitFailed(tr("Could not create a temporary folder in %1: %2")
                       .arg(runtimesRoot(), m_tempDir->errorString()));
        return;
    }

    if (m_provider == Provider::Mojang)
    {
        setStatus(
            tr("Fetching file list for Java %1...").arg(m_release.version));
        m_data->clear();
        auto dl = Net::Download::makeByteArray(m_release.url, m_data.get());
        if (!m_release.sha1.isEmpty())
        {
            dl->addValidator(new Net::ChecksumValidator(
                QCryptographicHash::Sha1, m_release.sha1));
        }
        NetJob::Ptr job(
            new NetJob(tr("Java manifest"), APPLICATION->network()));
        job->addNetAction(dl);
        runJob(job, &InstallTask::downloadMojangFiles);
        return;
    }
    downloadArchive();
}

void InstallTask::downloadArchive()
{
    setStatus(tr("Downloading %1 Java %2...")
                  .arg(providerName(m_provider), m_release.version));
    auto archive = m_tempDir->filePath("archive");
    auto dl = Net::Download::makeFile(m_release.url, archive);
    if (!m_release.sha256.isEmpty())
    {
        dl->addValidator(new Net::ChecksumValidator(QCryptographicHash::Sha256,
                                                    m_release.sha256));
    }
    NetJob::Ptr job(new NetJob(tr("Java download"), APPLICATION->network()));
    job->addNetAction(dl);
    runJob(job, &InstallTask::extract);
}

void InstallTask::extract()
{
    setStatus(tr("Extracting Java %1...").arg(m_release.version));
    setAbortable(false);
    m_extractWatcher.setFuture(QtConcurrent::run(ArchiveExtractor::extract,
                                                 m_tempDir->filePath("archive"),
                                                 m_tempDir->filePath("files")));
}

void InstallTask::extractFinished()
{
    setAbortable(true);
    auto error = m_extractWatcher.result();
    QFile::remove(m_tempDir->filePath("archive"));
    if (!error.isEmpty())
    {
        emitFailed(error);
        return;
    }
    install();
}

void InstallTask::downloadMojangFiles()
{
    auto files = parseJson(*m_data).object().value("files").toObject();
    if (files.isEmpty())
    {
        emitFailed(tr("The Mojang Java manifest is empty."));
        return;
    }

    QDir root(m_tempDir->filePath("files"));
    root.mkpath(".");

    m_links.clear();
    m_executables.clear();

    NetJob::Ptr job(
        new NetJob(tr("Java download"), APPLICATION->network(), nullptr, 16));
    for (auto it = files.begin(); it != files.end(); ++it)
    {
        auto path = pathInside(root, it.key());
        if (path.isEmpty())
        {
            emitFailed(
                tr("The Mojang Java manifest contains an invalid path: %1")
                    .arg(it.key()));
            return;
        }
        auto entry = it.value().toObject();
        auto type = entry.value("type").toString();
        if (type == "directory")
        {
            root.mkpath(path);
        }
        else if (type == "file")
        {
            auto raw =
                entry.value("downloads").toObject().value("raw").toObject();
            auto url = raw.value("url").toString();
            if (url.isEmpty())
            {
                emitFailed(tr("The Mojang Java manifest has no download for %1")
                               .arg(it.key()));
                return;
            }
            QDir().mkpath(QFileInfo(path).absolutePath());
            auto dl = Net::Download::makeFile(url, path);
            dl->addValidator(new Net::ChecksumValidator(
                QCryptographicHash::Sha1,
                QByteArray::fromHex(raw.value("sha1").toString().toLatin1())));
            job->addNetAction(dl);
            if (entry.value("executable").toBool())
            {
                m_executables.append(path);
            }
        }
        else if (type == "link")
        {
            auto target = entry.value("target").toString();
            auto resolved =
                QFileInfo(path).absoluteDir().absoluteFilePath(target);
            if (target.isEmpty() || QDir::isAbsolutePath(target) ||
                pathInside(root, root.relativeFilePath(resolved)).isEmpty())
            {
                emitFailed(
                    tr("The Mojang Java manifest contains an invalid link: %1")
                        .arg(it.key()));
                return;
            }
            m_links.append({path, target});
        }
    }

    setStatus(tr("Downloading Mojang Java %1...").arg(m_release.version));
    runJob(job, &InstallTask::finishMojangFiles);
}

void InstallTask::finishMojangFiles()
{
    for (auto& path : m_executables)
    {
        makeExecutable(path);
    }
    for (auto& link : m_links)
    {
        QDir().mkpath(QFileInfo(link.path).absolutePath());
        if (!QFile::link(link.target, link.path))
        {
            qWarning() << "Could not create link" << link.path << "->"
                       << link.target;
        }
    }
    install();
}

void InstallTask::install()
{
    setStatus(tr("Installing Java %1...").arg(m_release.version));

    // archives usually wrap everything in a single folder, use that as the
    // runtime root
    QString source = m_tempDir->filePath("files");
    auto entries = QDir(source).entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    if (entries.size() == 1 && entries[0].isDir() && !entries[0].isSymLink())
    {
        source = entries[0].absoluteFilePath();
    }

    if (findJavaBinary(source).isEmpty())
    {
        emitFailed(tr("The downloaded Java %1 does not contain %2.")
                       .arg(m_release.version, javaBinaryName()));
        return;
    }

    QDir root(runtimesRoot());
    auto folder =
        runtimeFolderName(m_provider, m_release.type, m_release.version,
                          m_release.mojangComponent);
    auto target = root.absoluteFilePath(folder);
    if (QFileInfo::exists(target) && !FS::deletePath(target))
    {
        emitFailed(tr("Could not replace %1. Is it still in use?").arg(target));
        return;
    }
    if (!moveDirectory(source, target))
    {
        emitFailed(tr("Could not move Java into %1").arg(target));
        return;
    }

    m_result.provider = m_provider;
    m_result.major = m_release.major;
    m_result.version = m_release.version;
    m_result.arch = hostArch();
    m_result.type = m_release.type;
    m_result.directory = target;
    m_result.javaPath = findJavaBinary(target);
#ifndef Q_OS_WIN
    for (auto& entry : QDir(QFileInfo(m_result.javaPath).absolutePath())
                           .entryInfoList(QDir::Files))
    {
        makeExecutable(entry.absoluteFilePath());
    }
#endif

    if (!writeMetadata(m_result))
    {
        emitFailed(tr("Could not write the metadata of Java %1.")
                       .arg(m_release.version));
        return;
    }
    m_tempDir.reset();
    emitSucceeded();
}

}  // namespace JavaRuntime
