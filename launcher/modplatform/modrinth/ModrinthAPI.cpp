#include "ModrinthAPI.h"

#include "Application.h"
#include "Json.h"
#include "modplatform/ModIndex.h"
#include "modplatform/modrinth/ModrinthPackIndex.h"
#include "net/Upload.h"

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

    QJsonObject body_obj;

    Json::writeStringList(body_obj, "hashes", hashes);
    Json::writeString(body_obj, "algorithm", hash_format);

    QJsonDocument body(body_obj);
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

    QJsonObject body_obj;

    Json::writeStringList(body_obj, "loaders", getModLoaderStrings(loaders));

    QStringList game_versions;
    for (auto& ver : mcVersions) {
        game_versions.append(ver.toString());
    }
    Json::writeStringList(body_obj, "game_versions", game_versions);

    QJsonDocument body(body_obj);
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

    QJsonObject body_obj;

    Json::writeStringList(body_obj, "hashes", hashes);
    Json::writeString(body_obj, "algorithm", hash_format);

    Json::writeStringList(body_obj, "loaders", getModLoaderStrings(loaders));

    QStringList game_versions;
    for (auto& ver : mcVersions) {
        game_versions.append(ver.toString());
    }
    Json::writeStringList(body_obj, "game_versions", game_versions);

    QJsonDocument body(body_obj);
    auto body_raw = body.toJson();

    netJob->addNetAction(Net::Upload::makeByteArray(QString(BuildConfig.MODRINTH_PROD_URL + "/version_files/update"), response, body_raw));

    QObject::connect(netJob, &NetJob::finished, [response] { delete response; });

    return netJob;
}

// TODO: dedup between?
ModPlatform::IndexedVersion ModrinthAPI::getLatestVersion(VersionSearchArgs&& args) const
{
    QEventLoop loop;

    auto netJob = new NetJob(QString("Modrinth::GetLatestVersion(%1)").arg(args.addonId), APPLICATION->network());
    auto response = new QByteArray();
    ModPlatform::IndexedVersion ver;

    netJob->addNetAction(Net::Download::makeByteArray(getVersionsURL(args), response));

    QObject::connect(netJob, &NetJob::succeeded, [response, args, &ver] {
        QJsonParseError parse_error{};
        QJsonDocument doc = QJsonDocument::fromJson(*response, &parse_error);
        if (parse_error.error != QJsonParseError::NoError) {
            qWarning() << "Error while parsing JSON response from latest mod version at " << parse_error.offset
                       << " reason: " << parse_error.errorString();
            qWarning() << *response;
            return;
        }

        try {
            auto arr = Json::requireArray(doc);

            QJsonObject latest_file_obj;
            ModPlatform::IndexedVersion ver_tmp;

            for (auto file : std::as_const(arr)) {
                auto file_obj = Json::requireObject(file);
                auto file_tmp = Modrinth::loadIndexedPackVersion(file_obj);
                if(file_tmp.date > ver_tmp.date) {
                    ver_tmp = file_tmp;
                    latest_file_obj = file_obj;
                }
            }

            ver = Modrinth::loadIndexedPackVersion(latest_file_obj);
        } catch (Json::JsonException& e) {
            qCritical() << "Failed to parse response from a version request.";
            qCritical() << e.what();
            qDebug() << doc;
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

auto ModrinthAPI::getProjects(QStringList addonIds, QByteArray* response) const -> NetJob*
{
    auto netJob = new NetJob(QString("Modrinth::GetProjects"), APPLICATION->network());
    auto searchUrl = getMultipleModInfoURL(addonIds);

    netJob->addNetAction(Net::Download::makeByteArray(QUrl(searchUrl), response));

    QObject::connect(netJob, &NetJob::finished, [response, netJob] { delete response; netJob->deleteLater(); });

    return netJob;
}
