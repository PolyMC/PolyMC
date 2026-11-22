#include "ConfigureLoki.h"

#include <launch/LaunchTask.h>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QUrl>
#include <Qt>
#include <utility>

#include "Application.h"
#include "net/Download.h"
#include "net/HttpMetaCache.h"
#include "net/NetAction.h"

ConfigureLoki::ConfigureLoki(LaunchTask* parent, QString yggdrasil_base_url,
                             std::shared_ptr<QString> javaagent_arg)
    : LaunchStep(parent),
      m_javaagent_arg{javaagent_arg},
      m_yggdrasil_base_url{yggdrasil_base_url}
{
}

void ConfigureLoki::executeTask()
{
    m_entry = APPLICATION->metacache()->resolveEntry("loki", "index.json");
    m_entry->setStale(true);

    m_job = std::make_unique<NetJob>("Download Loki index.json",
                                     APPLICATION->network());
    auto indexDl = Net::Download::makeCached(
        QUrl("https://meta.unmojang.org/v1/org.unmojang.loki/index.json"),
        m_entry, Net::Download::Option::NoOptions);
    m_job->addNetAction(indexDl);

    connect(m_job.get(), &NetJob::succeeded, this,
            &ConfigureLoki::indexDownloaded, Qt::QueuedConnection);

    connect(m_job.get(), &NetJob::failed, this, &ConfigureLoki::downloadFailed);
    m_job->start();
}

void ConfigureLoki::downloadFailed(QString reason)
{
    return emitFailed(QString("Download failed: %1").arg(reason));
}

void ConfigureLoki::indexDownloaded()
{
    QFile indexFile{m_entry->getFullPath()};
    if (!indexFile.open(QIODevice::ReadOnly))
    {
        return emitFailed(QString("Failed to open Loki index json: %1")
                              .arg(indexFile.errorString()));
    }

    QJsonParseError parseError;
    QJsonDocument indexDoc =
        QJsonDocument::fromJson(indexFile.readAll(), &parseError);
    indexFile.close();

    if (parseError.error != QJsonParseError::NoError || !indexDoc.isObject())
    {
        return emitFailed(QString("Failed to parse Loki index json: %1")
                              .arg(parseError.errorString()));
    }

    QJsonArray versions = indexDoc.object().value("versions").toArray();
    if (versions.isEmpty())
    {
        return emitFailed("Failed to parse Loki index json: no versions found");
    }

    QString selectedVersion;
    for (const QJsonValue& version : std::as_const(versions))
    {
        if (!version.isObject())
        {
            continue;
        }

        if (version.toObject().value("recommended").toBool())
        {
            selectedVersion = version.toObject().value("version").toString();
            break;
        }
    }

    if (selectedVersion.isEmpty())
    {
        qDebug() << "Failed to find recommended Loki version, falling "
                    "back to latest";
        selectedVersion =
            versions.first().toObject().value("version").toString();
    }

    m_entry = APPLICATION->metacache()->resolveEntry(
        "loki", QString("%1.json").arg(selectedVersion));
    m_selected_version = selectedVersion;
    m_job = std::make_unique<NetJob>("Download Loki version json",
                                     APPLICATION->network());
    auto versionDl = Net::Download::makeCached(
        QUrl(QString("https://meta.unmojang.org/v1/org.unmojang.loki/"
                     "%1.json")
                 .arg(selectedVersion)),
        m_entry, Net::Download::Option::NoOptions);
    m_job->addNetAction(versionDl);

    connect(m_job.get(), &NetJob::succeeded, this,
            &ConfigureLoki::versionDownloaded, Qt::QueuedConnection);

    connect(m_job.get(), &NetJob::failed, this, &ConfigureLoki::downloadFailed);
    m_job->start();
}

void ConfigureLoki::versionDownloaded()
{
    QFile versionFile{m_entry->getFullPath()};
    if (!versionFile.open(QIODevice::ReadOnly))
    {
        return emitFailed(QString("Failed to open Loki version json: %1")
                              .arg(versionFile.errorString()));
    }

    QJsonParseError versionParseError;
    QJsonDocument versionDoc =
        QJsonDocument::fromJson(versionFile.readAll(), &versionParseError);
    versionFile.close();

    if (versionParseError.error != QJsonParseError::NoError ||
        !versionDoc.isObject())
    {
        return emitFailed(QString("Failed to parse Loki version json: %1")
                              .arg(versionParseError.errorString()));
    }

    QJsonArray agents = versionDoc.object().value("+agents").toArray();
    if (agents.isEmpty())
    {
        return emitFailed(
            "Failed to parse Loki version json: '+agents' "
            "missing or empty");
    }

    QJsonObject agentObj = agents.first().toObject();
    QString lokiJarUrl = agentObj.value("MMC-absoluteUrl").toString();
    if (lokiJarUrl.isEmpty())
    {
        return emitFailed(
            "Failed to parse Loki version json: download url "
            "missing");
    }

    QString filename = QFileInfo(QUrl(lokiJarUrl).path()).fileName();
    if (filename.isEmpty())
    {
        filename = QString("Loki-%1.jar").arg(m_selected_version);
    }

    m_entry = APPLICATION->metacache()->resolveEntry("loki", filename);
    m_job = std::make_unique<NetJob>("Download Loki java agent",
                                     APPLICATION->network());
    auto javaAgentDl = Net::Download::makeCached(
        QUrl(lokiJarUrl), m_entry, Net::Download::Option::MakeEternal);

    m_job->addNetAction(javaAgentDl);

    connect(m_job.get(), &NetJob::succeeded, this,
            &ConfigureLoki::javaAgentDownloaded);
    connect(m_job.get(), &NetJob::failed, this, &ConfigureLoki::downloadFailed);
    m_job->start();
}

void ConfigureLoki::javaAgentDownloaded()
{
    auto path = m_entry->getFullPath();
    qDebug() << "Configured Loki agent:" << path;
    *m_javaagent_arg = QString("%1=%2").arg(path, m_yggdrasil_base_url);
    emitSucceeded();
}

void ConfigureLoki::finalize()
{
}

#include "moc_ConfigureLoki.cpp"
