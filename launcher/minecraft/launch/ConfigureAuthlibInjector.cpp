#include "ConfigureAuthlibInjector.h"
#include <launch/LaunchTask.h>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <Qt>

#include "Application.h"
#include "minecraft/auth/AccountList.h"
#include "net/ChecksumValidator.h"
#include "net/Download.h"
#include "net/HttpMetaCache.h"
#include "net/NetAction.h"

ConfigureAuthlibInjector::ConfigureAuthlibInjector(LaunchTask* parent,
                                                   QString authlibinjector_base_url,
                                                   std::shared_ptr<QString> javaagent_arg)
    : LaunchStep(parent), m_javaagent_arg{ javaagent_arg }, m_authlibinjector_base_url{ authlibinjector_base_url }
{}

void ConfigureAuthlibInjector::executeTask()
{
    auto downloadFailed = [this] (QString reason) {
        return emitFailed(QString("Download failed: %1").arg(reason));
    };
    auto entry = APPLICATION->metacache()->resolveEntry("authlibinjector", "latest.json");

    entry->setStale(true);
    m_job = std::make_unique<NetJob>("Download authlibinjector latest.json", APPLICATION->network());
    auto latestJsonDl =
        Net::Download::makeCached(QUrl("https://authlib-injector.yushi.moe/artifact/latest.json"), entry, Net::Download::Option::NoOptions);
    m_job->addNetAction(latestJsonDl);
    connect(m_job.get(), &NetJob::succeeded, this, [this, entry, downloadFailed] {
        QFile authlibInjectorLatestJson = entry->getFullPath();
        authlibInjectorLatestJson.open(QIODevice::ReadOnly);
        if (!authlibInjectorLatestJson.isOpen())
            return emitFailed(QString("Failed to open authlib-injector info json: %1").arg(authlibInjectorLatestJson.errorString()));

        QJsonParseError json_parse_error;
        QJsonDocument doc = QJsonDocument::fromJson(authlibInjectorLatestJson.readAll(), &json_parse_error);
        if (json_parse_error.error != QJsonParseError::NoError)
            return emitFailed(QString("Failed to parse authlib-injector info json: %1").arg(json_parse_error.errorString()));

        if (!doc.isObject())
            return emitFailed(QString("Failed to parse authlib-injector info json: not a json object"));
        QJsonObject obj = doc.object();

        QString authlibInjectorJarUrl = obj["download_url"].toString();
        if (authlibInjectorJarUrl.isNull())
            return emitFailed(QString("Failed to parse authlib-injector info json: download url missing"));

        QString sha256Sum = obj["checksums"].toObject()["sha256"].toString();
        if (sha256Sum.isNull())
            return emitFailed("Failed to parse authlib-injector info json: sha256 checksum missing");

        auto sha256SumRaw = QByteArray::fromHex(sha256Sum.toLatin1());

        QString filename = QFileInfo(authlibInjectorJarUrl).fileName();
        auto javaAgentEntry = APPLICATION->metacache()->resolveEntry("authlibinjector", filename);
        m_job = std::make_unique<NetJob>("Download authlibinjector java agent", APPLICATION->network());
        auto javaAgentDl = Net::Download::makeCached(QUrl(authlibInjectorJarUrl), javaAgentEntry, Net::Download::Option::MakeEternal);
        javaAgentDl->addValidator(new Net::ChecksumValidator(QCryptographicHash::Sha256, sha256SumRaw));
        m_job->addNetAction(javaAgentDl);
        connect(m_job.get(), &NetJob::succeeded, this, [this, javaAgentEntry] {
            auto path = javaAgentEntry->getFullPath();
            qDebug() << path;
            *m_javaagent_arg = QString("%1=%2").arg(path).arg(m_authlibinjector_base_url);
            emitSucceeded();
        });
        connect(m_job.get(), &NetJob::failed, this, downloadFailed);
        m_job->start();
    },
    // This slot can't run instantly because it needs to wait for the netjob's code to stop running
    // Since it will destroy the old netjob by reassigning the unique_ptr
    Qt::QueuedConnection);
    connect(m_job.get(), &NetJob::failed, this, downloadFailed);
    m_job->start();
}

void ConfigureAuthlibInjector::finalize() {}
