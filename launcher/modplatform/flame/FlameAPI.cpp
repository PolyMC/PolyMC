#include "FlameAPI.h"
#include "FlameModIndex.h"

#include "Application.h"
#include "BuildConfig.h"
#include "Json.h"

#include "net/Upload.h"

auto FlameAPI::matchFingerprints(const QList<uint>& fingerprints, QByteArray* response) -> NetJob::Ptr
{
    auto* netJob = new NetJob(QString("Flame::MatchFingerprints"), APPLICATION->network());

    nlohmann::json body_obj;
    for (auto& fp : fingerprints) {
        body_obj["fingerprints"].push_back(fp);
    }

    QString body_str = body_obj.dump().c_str();

    netJob->addNetAction(Net::Upload::makeByteArray(QString("https://api.curseforge.com/v1/fingerprints"), response, body_str.toUtf8()));

    QObject::connect(netJob, &NetJob::finished, [response] { delete response; });

    return netJob;
}

auto FlameAPI::getModFileChangelog(int modId, int fileId) -> QString
{
    QEventLoop lock;
    QString changelog;

    auto* netJob = new NetJob(QString("Flame::FileChangelog"), APPLICATION->network());
    auto* response = new QByteArray();
    netJob->addNetAction(Net::Download::makeByteArray(
        QString("https://api.curseforge.com/v1/mods/%1/files/%2/changelog")
            .arg(QString::fromStdString(std::to_string(modId)), QString::fromStdString(std::to_string(fileId))),
        response));

    QObject::connect(netJob, &NetJob::succeeded, [netJob, response, &changelog] {
        nlohmann::json doc;
        try {
            doc = nlohmann::json::parse(response->constData(), response->constData() + response->size());
        }
        catch (const nlohmann::json::exception& e) {
            qWarning() << "Error while parsing JSON response from Flame::FileChangelog at " << e.what();
            qWarning() << *response;
            return;
        }

        changelog = doc["data"].get<std::string>().c_str();
    });

    QObject::connect(netJob, &NetJob::finished, [response, &lock] {
        delete response;
        lock.quit();
    });

    netJob->start();
    lock.exec();

    return changelog;
}

auto FlameAPI::getModDescription(int modId) -> QString
{
    QEventLoop lock;
    QString description;

    auto* netJob = new NetJob(QString("Flame::ModDescription"), APPLICATION->network());
    auto* response = new QByteArray();
    netJob->addNetAction(Net::Download::makeByteArray(
        QString("https://api.curseforge.com/v1/mods/%1/description")
            .arg(QString::number(modId)), response));

    QObject::connect(netJob, &NetJob::succeeded, [netJob, response, &description] {
        nlohmann::json doc;
        try {
            doc = nlohmann::json::parse(response->constData(), response->constData() + response->size());
        }
        catch (const nlohmann::json::exception& e) {
            qWarning() << "Error while parsing JSON response from Flame::ModDescription at " << e.what();
            qWarning() << *response;
            return;
        }

        description = doc.value("data", "").c_str();
    });

    QObject::connect(netJob, &NetJob::finished, [response, &lock] {
        delete response;
        lock.quit();
    });

    netJob->start();
    lock.exec();

    return description;
}

auto FlameAPI::getLatestVersion(VersionSearchArgs&& args) -> ModPlatform::IndexedVersion
{
    QEventLoop loop;

    auto netJob = new NetJob(QString("Flame::GetLatestVersion(%1)").arg(args.addonId), APPLICATION->network());
    auto response = new QByteArray();
    ModPlatform::IndexedVersion ver;

    netJob->addNetAction(Net::Download::makeByteArray(getVersionsURL(args), response));

    QObject::connect(netJob, &NetJob::succeeded, [response, args, &ver] {
        nlohmann::json doc;
        try {
            doc = nlohmann::json::parse(response->constData(), response->constData() + response->size());
        } catch (nlohmann::json::parse_error& e) {
            qWarning() << "Error while parsing JSON response from latest mod version at " << e.byte << " reason: " << e.what();
            qWarning() << *response;
            return;
        }

        try {
            const auto& arr = doc["data"];

            nlohmann::json latest_file_obj;
            ModPlatform::IndexedVersion ver_tmp;

            for (const auto& file_obj : arr) {
                auto file_tmp = FlameMod::loadIndexedPackVersion(file_obj);
                if(file_tmp.date > ver_tmp.date) {
                    ver_tmp = file_tmp;
                    latest_file_obj = file_obj;
                }
            }

            ver = FlameMod::loadIndexedPackVersion(latest_file_obj);
        } catch (const std::exception& e) {
            qCritical() << "Failed to parse response from a version request.";
            qCritical() << e.what();
            qDebug() << doc.dump().c_str();
        }
    });

    QObject::connect(netJob, &NetJob::finished, [response, netJob, &loop] {
        netJob->deleteLater();
        delete response;
        loop.quit();
    });

    netJob->start();

    loop.exec();

    return ver;
}

auto FlameAPI::getProjects(QStringList addonIds, QByteArray* response) const -> NetJob*
{
    auto* netJob = new NetJob(QString("Flame::GetProjects"), APPLICATION->network());

    nlohmann::json body_obj;
    for (const auto& addonId : addonIds) {
        body_obj["modIds"].push_back(addonId.toStdString());
    }

    QString body_str = body_obj.dump().c_str();

    netJob->addNetAction(Net::Upload::makeByteArray(QString("https://api.curseforge.com/v1/mods"), response, body_str.toUtf8()));

    QObject::connect(netJob, &NetJob::finished, [response, netJob] { delete response; netJob->deleteLater(); });
    QObject::connect(netJob, &NetJob::failed, [body_str] { qDebug() << body_str; });

    return netJob;
}

auto FlameAPI::getFiles(const QStringList& fileIds, QByteArray* response) const -> NetJob*
{
    auto* netJob = new NetJob(QString("Flame::GetFiles"), APPLICATION->network());

    nlohmann::json body_obj;
    for (const auto& fileId : fileIds) {
        body_obj["fileIds"].push_back(fileId.toStdString());
    }

    QString body_str = body_obj.dump().c_str();

    netJob->addNetAction(Net::Upload::makeByteArray(QString("https://api.curseforge.com/v1/mods/files"), response, body_str.toUtf8()));

    QObject::connect(netJob, &NetJob::finished, [response, netJob] { delete response; netJob->deleteLater(); });
    QObject::connect(netJob, &NetJob::failed, [body_str] { qDebug() << body_str; });

    return netJob;
}
