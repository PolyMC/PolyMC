#include "ConfigureLoki.h"
#include <launch/LaunchTask.h>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <Qt>

#include "Application.h"
#include "minecraft/auth/AccountList.h"
#include "net/ChecksumValidator.h"
#include "net/Download.h"
#include "net/HttpMetaCache.h"
#include "net/NetAction.h"

ConfigureLoki::ConfigureLoki(LaunchTask* parent, QString yggdrasil_base_url, std::shared_ptr<QString> javaagent_arg)
    : LaunchStep(parent), m_javaagent_arg{ javaagent_arg }, m_yggdrasil_base_url{ yggdrasil_base_url }
{}

void ConfigureLoki::executeTask()
{
    auto downloadFailed = [this](QString reason) { return emitFailed(QString("Download failed: %1").arg(reason)); };

    auto indexEntry = APPLICATION->metacache()->resolveEntry("loki", "index.json");
    indexEntry->setStale(true);

    m_job = std::make_unique<NetJob>("Download Loki index.json", APPLICATION->network());
    auto indexDl = Net::Download::makeCached(QUrl("https://meta.unmojang.org/v1/org.unmojang.loki/index.json"), indexEntry,
                                             Net::Download::Option::NoOptions);
    m_job->addNetAction(indexDl);

    connect(
        m_job.get(), &NetJob::succeeded, this,
        [this, indexEntry, downloadFailed] {
            QFile indexFile{ indexEntry->getFullPath() };
            if (!indexFile.open(QIODevice::ReadOnly))
                return emitFailed(QString("Failed to open Loki index json: %1").arg(indexFile.errorString()));

            QJsonParseError parseError;
            QJsonDocument indexDoc = QJsonDocument::fromJson(indexFile.readAll(), &parseError);
            indexFile.close();

            if (parseError.error != QJsonParseError::NoError || !indexDoc.isObject())
                return emitFailed(QString("Failed to parse Loki index json: %1").arg(parseError.errorString()));

            QJsonArray versions = indexDoc.object()["versions"].toArray();
            if (versions.isEmpty())
                return emitFailed("Failed to parse Loki index json: no versions found");

            QString latestVersion = versions[0].toObject()["version"].toString();
            if (latestVersion.isEmpty())
                return emitFailed("Failed to parse Loki index json: invalid version entry");

            auto versionEntry = APPLICATION->metacache()->resolveEntry("loki", QString("%1.json").arg(latestVersion));
            m_job = std::make_unique<NetJob>("Download Loki version json", APPLICATION->network());
            auto versionDl =
                Net::Download::makeCached(QUrl(QString("https://meta.unmojang.org/v1/org.unmojang.loki/%1.json").arg(latestVersion)),
                                          versionEntry, Net::Download::Option::NoOptions);
            m_job->addNetAction(versionDl);

            connect(
                m_job.get(), &NetJob::succeeded, this,
                [this, versionEntry, latestVersion, downloadFailed] {
                    QFile versionFile{ versionEntry->getFullPath() };
                    if (!versionFile.open(QIODevice::ReadOnly))
                        return emitFailed(QString("Failed to open Loki version json: %1").arg(versionFile.errorString()));

                    QJsonParseError versionParseError;
                    QJsonDocument versionDoc = QJsonDocument::fromJson(versionFile.readAll(), &versionParseError);
                    versionFile.close();

                    if (versionParseError.error != QJsonParseError::NoError || !versionDoc.isObject())
                        return emitFailed(QString("Failed to parse Loki version json: %1").arg(versionParseError.errorString()));

                    QJsonArray agents = versionDoc.object()["+agents"].toArray();
                    if (agents.isEmpty())
                        return emitFailed("Failed to parse Loki version json: '+agents' missing or empty");

                    QJsonObject agentObj = agents[0].toObject();
                    QString lokiJarUrl = agentObj["MMC-absoluteUrl"].toString();
                    if (lokiJarUrl.isEmpty())
                        lokiJarUrl = agentObj["url"].toString();
                    if (lokiJarUrl.isEmpty())
                        lokiJarUrl = agentObj["download_url"].toString();
                    if (lokiJarUrl.isEmpty())
                        return emitFailed("Failed to parse Loki version json: download url missing");

                    QString sha256Sum = agentObj["checksums"].toObject()["sha256"].toString();
                    if (sha256Sum.isEmpty())
                        sha256Sum = agentObj["sha256"].toString();

                    QString filename = QFileInfo(QUrl(lokiJarUrl).path()).fileName();
                    if (filename.isEmpty())
                        filename = QString("loki-%1.jar").arg(latestVersion);

                    auto javaAgentEntry = APPLICATION->metacache()->resolveEntry("loki", filename);
                    m_job = std::make_unique<NetJob>("Download Loki java agent", APPLICATION->network());
                    auto javaAgentDl = Net::Download::makeCached(QUrl(lokiJarUrl), javaAgentEntry, Net::Download::Option::MakeEternal);

                    if (!sha256Sum.isEmpty()) {
                        auto sha256SumRaw = QByteArray::fromHex(sha256Sum.toLatin1());
                        javaAgentDl->addValidator(new Net::ChecksumValidator(QCryptographicHash::Sha256, sha256SumRaw));
                    }

                    m_job->addNetAction(javaAgentDl);

                    connect(m_job.get(), &NetJob::succeeded, this, [this, javaAgentEntry] {
                        auto path = javaAgentEntry->getFullPath();
                        qDebug() << "Configured Loki agent:" << path;
                        *m_javaagent_arg = QString("%1=%2").arg(path, m_yggdrasil_base_url);
                        emitSucceeded();
                    });
                    connect(m_job.get(), &NetJob::failed, this, downloadFailed);
                    m_job->start();
                },
                Qt::QueuedConnection);

            connect(m_job.get(), &NetJob::failed, this, downloadFailed);
            m_job->start();
        },
        Qt::QueuedConnection);

    connect(m_job.get(), &NetJob::failed, this, downloadFailed);
    m_job->start();
}

void ConfigureLoki::finalize() {}