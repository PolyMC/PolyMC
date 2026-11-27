// SPDX-License-Identifier: GPL-3.0-only

#include "ConfigureYggdrasilAgent.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <Qt>
#include <utility>

#include "Application.h"
#include "Exception.h"
#include "FileSystem.h"
#include "net/ChecksumValidator.h"

namespace
{
// Names the jar of the last launch that got one, for when no source can be
// reached
const QString yggdrasilLastAgentFile = "last-agent.txt";

QJsonObject parseAgentJson(const QByteArray& data, QString& error)
{
    QJsonParseError parse_error;
    auto doc = QJsonDocument::fromJson(data, &parse_error);
    if (parse_error.error != QJsonParseError::NoError || !doc.isObject())
    {
        error = parse_error.errorString();
        return {};
    }
    return doc.object();
}
}  // namespace

ConfigureYggdrasilAgent::ConfigureYggdrasilAgent(
    LaunchTask* parent, QString server, std::shared_ptr<QString> argument)
    : LaunchStep(parent),
      m_javaagent_arg(std::move(argument)),
      m_yggdrasil_base_url(std::move(server))
{
}

void ConfigureYggdrasilAgent::executeTask()
{
    m_sources = sources();
    m_source_index = 0;
    m_errors.clear();
    trySource();
}

void ConfigureYggdrasilAgent::finalize()
{
}

QUrl ConfigureYggdrasilAgent::metaUrlOverride()
{
    QUrl url(APPLICATION->settings()->get("MetaURLOverride").toString());
    // Component folders are resolved against it
    if (!url.isEmpty() && !url.path().endsWith('/'))
    {
        url.setPath(url.path() + '/');
    }
    return url;
}

void ConfigureYggdrasilAgent::trySource()
{
    if (m_source_index >= m_sources.size())
    {
        useLastJar();
        return;
    }

    const auto& source = m_sources.at(m_source_index);
    m_version.clear();
    // The index and latest.json say which jar is current, so they are always
    // asked for again
    if (source.component)
    {
        m_entry = APPLICATION->metacache()->resolveEntry(
            cacheBase(), source.cachePrefix + "index.json");
        m_entry->setStale(true);
        startDownload(QString("Download %1 index.json").arg(agentName()),
                      Net::Download::makeCached(
                          source.url.resolved(QUrl("index.json")), m_entry),
                      &ConfigureYggdrasilAgent::indexDownloaded);
    }
    else
    {
        m_entry = APPLICATION->metacache()->resolveEntry(
            cacheBase(), source.cachePrefix + "latest.json");
        m_entry->setStale(true);
        startDownload(QString("Download %1 latest.json").arg(agentName()),
                      Net::Download::makeCached(source.url, m_entry),
                      &ConfigureYggdrasilAgent::latestDownloaded);
    }
}

void ConfigureYggdrasilAgent::startDownload(
    const QString& name, Net::Download::Ptr download,
    void (ConfigureYggdrasilAgent::*next)())
{
    m_next = next;
    m_job = std::make_unique<NetJob>(name, APPLICATION->network());
    m_job->addNetAction(download);
    // Queued, because the next step replaces the job, which must not happen
    // while it is still emitting
    connect(m_job.get(), &NetJob::succeeded, this,
            &ConfigureYggdrasilAgent::jobSucceeded, Qt::QueuedConnection);
    connect(m_job.get(), &NetJob::failed, this,
            &ConfigureYggdrasilAgent::sourceFailed, Qt::QueuedConnection);
    m_job->start();
}

void ConfigureYggdrasilAgent::jobSucceeded()
{
    (this->*m_next)();
}

void ConfigureYggdrasilAgent::sourceFailed(QString reason)
{
    auto url = m_sources.at(m_source_index).url.toString();
    qWarning() << "Couldn't get" << agentName() << "from" << url << ":"
               << reason;
    m_errors.append(QString("%1: %2").arg(url, reason));
    m_source_index++;
    trySource();
}

QByteArray ConfigureYggdrasilAgent::readEntry()
{
    QFile file(m_entry->getFullPath());
    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }
    return file.readAll();
}

void ConfigureYggdrasilAgent::indexDownloaded()
{
    QString error;
    auto versions =
        parseAgentJson(readEntry(), error).value("versions").toArray();
    if (versions.isEmpty())
    {
        sourceFailed(error.isEmpty() ? QString("The index lists no versions")
                                     : error);
        return;
    }

    for (const auto& value : versions)
    {
        auto version = value.toObject();
        if (version.value("recommended").toBool())
        {
            m_version = version.value("version").toString();
            break;
        }
    }
    if (m_version.isEmpty())
    {
        // Newest first
        m_version = versions.first().toObject().value("version").toString();
    }

    const auto& source = m_sources.at(m_source_index);
    m_entry = APPLICATION->metacache()->resolveEntry(
        cacheBase(), source.cachePrefix + m_version + ".json");
    startDownload(QString("Download %1 version json").arg(agentName()),
                  Net::Download::makeCached(
                      source.url.resolved(QUrl(m_version + ".json")), m_entry),
                  &ConfigureYggdrasilAgent::versionDownloaded);
}

void ConfigureYggdrasilAgent::versionDownloaded()
{
    QString error;
    auto agents = parseAgentJson(readEntry(), error).value("+agents").toArray();
    if (agents.isEmpty())
    {
        sourceFailed(error.isEmpty() ? QString("The version lists no agent")
                                     : error);
        return;
    }

    // Mirrors may give the jar as a library download with a checksum
    auto agent = agents.first().toObject();
    auto artifact =
        agent.value("downloads").toObject().value("artifact").toObject();
    auto url = artifact.value("url").toString();
    if (url.isEmpty())
    {
        url = agent.value("MMC-absoluteUrl").toString();
    }
    if (url.isEmpty())
    {
        url = agent.value("url").toString();
    }
    if (url.isEmpty())
    {
        sourceFailed("The agent has no download URL");
        return;
    }
    downloadJar(
        QUrl(url), QCryptographicHash::Sha1,
        QByteArray::fromHex(artifact.value("sha1").toString().toLatin1()));
}

void ConfigureYggdrasilAgent::latestDownloaded()
{
    QString error;
    auto latest = parseAgentJson(readEntry(), error);
    auto url = latest.value("download_url").toString();
    auto sha256 =
        latest.value("checksums").toObject().value("sha256").toString();
    m_version = latest.value("version").toString();
    if (url.isEmpty() || sha256.isEmpty())
    {
        sourceFailed(error.isEmpty()
                         ? QString("The download URL or checksum is missing")
                         : error);
        return;
    }
    downloadJar(QUrl(url), QCryptographicHash::Sha256,
                QByteArray::fromHex(sha256.toLatin1()));
}

void ConfigureYggdrasilAgent::downloadJar(
    const QUrl& url, QCryptographicHash::Algorithm algorithm,
    const QByteArray& checksum)
{
    // Every source names the jar the same, so a jar cached from one is reused
    // for the others
    auto filename = QFileInfo(url.path()).fileName();
    if (filename.isEmpty())
    {
        filename = QString("%1-%2.jar").arg(agentName(), m_version);
    }
    m_entry = APPLICATION->metacache()->resolveEntry(cacheBase(), filename);
    auto download = Net::Download::makeCached(
        url, m_entry, Net::Download::Option::MakeEternal);
    if (!checksum.isEmpty())
    {
        download->addValidator(new Net::ChecksumValidator(algorithm, checksum));
    }
    startDownload(QString("Download %1 java agent").arg(agentName()), download,
                  &ConfigureYggdrasilAgent::jarDownloaded);
}

void ConfigureYggdrasilAgent::jarDownloaded()
{
    auto path = m_entry->getFullPath();
    try
    {
        FS::write(
            FS::PathCombine(APPLICATION->metacache()->getBasePath(cacheBase()),
                            yggdrasilLastAgentFile),
            QFileInfo(path).fileName().toUtf8());
    }
    catch (const Exception& e)
    {
        qWarning() << "Couldn't remember the" << agentName()
                   << "jar:" << e.cause();
    }
    configure(path);
}

void ConfigureYggdrasilAgent::useLastJar()
{
    auto base = APPLICATION->metacache()->getBasePath(cacheBase());
    QString filename;
    QFile last(FS::PathCombine(base, yggdrasilLastAgentFile));
    if (last.open(QIODevice::ReadOnly))
    {
        filename =
            QFileInfo(QString::fromUtf8(last.readAll()).trimmed()).fileName();
    }

    auto path = FS::PathCombine(base, filename);
    if (!filename.isEmpty() && QFileInfo(path).size() > 0)
    {
        qWarning() << "No source of" << agentName()
                   << "could be reached, using the jar of an earlier launch:"
                   << path;
        emit logLine(tr("Failed to download %1, using the copy from a "
                        "previous launch.\n")
                         .arg(agentName()),
                     MessageLevel::Warning);
        configure(path);
        return;
    }
    auto reason =
        tr("Failed to download %1:\n%2").arg(agentName(), m_errors.join("\n"));
    emit logLine(reason, MessageLevel::Error);
    emitFailed(reason);
}

void ConfigureYggdrasilAgent::configure(const QString& path)
{
    qDebug() << "Configured" << agentName() << "agent:" << path;
    *m_javaagent_arg = QString("%1=%2").arg(path, m_yggdrasil_base_url);
    emitSucceeded();
}

#include "moc_ConfigureYggdrasilAgent.cpp"
