#include "ModrinthAPI.h"

#include "Application.h"
#include "net/Upload.h"

#include "json.hpp"

auto ModrinthAPI::currentVersion(QString hash, QString hash_format, QByteArray* response) -> NetJob::Ptr
{
    auto* netJob = new NetJob(QString("Modrinth::GetCurrentVersion"), APPLICATION->network());

    netJob->addNetAction(Net::Download::makeByteArray(
        QString(BuildConfig.MODRINTH_PROD_URL + "/version_file/%1?algorithm=%2").arg(hash, hash_format), response));

    QObject::connect(netJob, &NetJob::finished, [response] { delete response; });

    return netJob;
}

auto ModrinthAPI::currentVersions(const QStringList& hashes, QString hash_format, QByteArray* response) -> NetJob::Ptr
{
    auto* netJob = new NetJob(QString("Modrinth::GetCurrentVersions"), APPLICATION->network());

    nlohmann::json body_obj;

    for (auto& hash : hashes) {
        body_obj["hashes"].push_back(hash.toStdString());
    }
    body_obj["algorithm"] = hash_format.toStdString();

    QString body_str = body_obj.dump().c_str();
    QJsonDocument body = QJsonDocument::fromJson(body_str.toUtf8());
    auto body_raw = body.toJson();

    netJob->addNetAction(Net::Upload::makeByteArray(QString(BuildConfig.MODRINTH_PROD_URL + "/version_files"), response, body_raw));

    QObject::connect(netJob, &NetJob::finished, [response] { delete response; });

    return netJob;
}

auto ModrinthAPI::latestVersion(QString hash,
                                QString hash_format,
                                std::list<Version> mcVersions,
                                ModLoaderTypes loaders,
                                QByteArray* response) -> NetJob::Ptr
{
    auto* netJob = new NetJob(QString("Modrinth::GetLatestVersion"), APPLICATION->network());

    nlohmann::json body_obj;

    for (auto& loader : getModLoaderStrings(loaders)) {
        body_obj["loaders"].push_back(loader.toStdString());
    }

    for (auto& ver : mcVersions) {
        body_obj["game_versions"].push_back(ver.toString().toStdString());
    }

    QString body_str = body_obj.dump().c_str();
    QJsonDocument body = QJsonDocument::fromJson(body_str.toUtf8());
    auto body_raw = body.toJson();


    netJob->addNetAction(Net::Upload::makeByteArray(
        QString(BuildConfig.MODRINTH_PROD_URL + "/version_file/%1/update?algorithm=%2").arg(hash, hash_format), response, body_raw));

    QObject::connect(netJob, &NetJob::finished, [response] { delete response; });

    return netJob;
}

auto ModrinthAPI::latestVersions(const QStringList& hashes,
                                 QString hash_format,
                                 std::list<Version> mcVersions,
                                 ModLoaderTypes loaders,
                                 QByteArray* response) -> NetJob::Ptr
{
    auto* netJob = new NetJob(QString("Modrinth::GetLatestVersions"), APPLICATION->network());

    nlohmann::json body_obj;

    for (auto& hash : hashes) {
        body_obj["hashes"].push_back(hash.toStdString());
    }

    body_obj["algorithm"] = hash_format.toStdString();

    for (auto& loader : getModLoaderStrings(loaders)) {
        body_obj["loaders"].push_back(loader.toStdString());
    }

    for (auto& ver : mcVersions) {
        body_obj["game_versions"].push_back(ver.toString().toStdString());
    }

    QString body_str = body_obj.dump().c_str();
    QJsonDocument body = QJsonDocument::fromJson(body_str.toUtf8());
    auto body_raw = body.toJson();

    netJob->addNetAction(Net::Upload::makeByteArray(QString(BuildConfig.MODRINTH_PROD_URL + "/version_files/update"), response, body_raw));

    QObject::connect(netJob, &NetJob::finished, [response] { delete response; });

    return netJob;
}

auto ModrinthAPI::getProjects(QStringList addonIds, QByteArray* response) const -> NetJob*
{
    auto netJob = new NetJob(QString("Modrinth::GetProjects"), APPLICATION->network());
    auto searchUrl = getMultipleModInfoURL(addonIds);

    netJob->addNetAction(Net::Download::makeByteArray(QUrl(searchUrl), response));

    QObject::connect(netJob, &NetJob::finished, [response, netJob] { delete response; netJob->deleteLater(); });

    return netJob;
}
