#include "FlameAPI.h"
#include "FlameModIndex.h"

#include "Application.h"
#include "BuildConfig.h"
#include "Json.h"

#include "net/Upload.h"

#include <functional>

namespace
{

void deleteResponse(QByteArray* response)
{
    delete response;
}

void storeChangelog(QByteArray* response, QString& changelog)
{
    changelog = FlameAPI::parseModFileChangelog(*response);
}

void finishBlockingJob(NetJob* job, QByteArray* response, QEventLoop& loop)
{
    // Nothing else owns the job
    job->deleteLater();
    delete response;
    loop.quit();
}

void storeDescription(NetJob* job, QByteArray* response, QString& description)
{
    QJsonParseError parse_error{};
    QJsonDocument doc = QJsonDocument::fromJson(*response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from "
                      "Flame::ModDescription at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << *response;

        job->failed(parse_error.errorString());
        return;
    }

    description = Json::ensureString(doc.object(), "data");
}

void storeLatestVersion(QByteArray* response,
                        ModPlatform::IndexedVersion& version)
{
    version = FlameAPI::parseLatestVersion(*response);
}

void releaseJob(QByteArray* response, NetJob* job)
{
    delete response;
    job->deleteLater();
}

void logBody(const QByteArray& body)
{
    qDebug() << body;
}

}  // namespace

NetJob::Ptr FlameAPI::matchFingerprints(const QList<uint>& fingerprints,
                                        QByteArray* response)
{
    auto* netJob =
        new NetJob(QString("Flame::MatchFingerprints"), APPLICATION->network());

    QJsonObject body_obj;
    QJsonArray fingerprints_arr;
    for (auto& fp : fingerprints)
    {
        fingerprints_arr.append(QString("%1").arg(fp));
    }

    body_obj["fingerprints"] = fingerprints_arr;

    QJsonDocument body(body_obj);
    auto body_raw = body.toJson();

    netJob->addNetAction(Net::Upload::makeByteArray(
        QString("https://api.curseforge.com/v1/fingerprints"), response,
        body_raw));

    QObject::connect(netJob, &NetJob::finished,
                     std::bind(deleteResponse, response));

    return netJob;
}

QString FlameAPI::getModFileChangelogURL(int modId, int fileId) const
{
    return QString("https://api.curseforge.com/v1/mods/%1/files/%2/changelog")
        .arg(QString::fromStdString(std::to_string(modId)),
             QString::fromStdString(std::to_string(fileId)));
}

QString FlameAPI::parseModFileChangelog(const QByteArray& response)
{
    QJsonParseError parse_error{};
    QJsonDocument doc = QJsonDocument::fromJson(response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning()
            << "Error while parsing JSON response from Flame::FileChangelog at "
            << parse_error.offset << " reason: " << parse_error.errorString();
        qWarning() << response;
        return {};
    }

    return Json::ensureString(doc.object(), "data");
}

QString FlameAPI::getModFileChangelog(int modId, int fileId)
{
    QEventLoop lock;
    QString changelog;

    auto* netJob =
        new NetJob(QString("Flame::FileChangelog"), APPLICATION->network());
    auto* response = new QByteArray();
    netJob->addNetAction(Net::Download::makeByteArray(
        getModFileChangelogURL(modId, fileId), response));

    QObject::connect(netJob, &NetJob::succeeded,
                     std::bind(storeChangelog, response, std::ref(changelog)));

    QObject::connect(
        netJob, &NetJob::finished,
        std::bind(finishBlockingJob, netJob, response, std::ref(lock)));

    netJob->start();
    lock.exec();

    return changelog;
}

QString FlameAPI::getModDescription(int modId)
{
    QEventLoop lock;
    QString description;

    auto* netJob =
        new NetJob(QString("Flame::ModDescription"), APPLICATION->network());
    auto* response = new QByteArray();
    netJob->addNetAction(Net::Download::makeByteArray(
        QString("https://api.curseforge.com/v1/mods/%1/description")
            .arg(QString::number(modId)),
        response));

    QObject::connect(
        netJob, &NetJob::succeeded,
        std::bind(storeDescription, netJob, response, std::ref(description)));

    QObject::connect(
        netJob, &NetJob::finished,
        std::bind(finishBlockingJob, netJob, response, std::ref(lock)));

    netJob->start();
    lock.exec();

    return description;
}

ModPlatform::IndexedVersion FlameAPI::parseLatestVersion(
    const QByteArray& response)
{
    ModPlatform::IndexedVersion ver;

    QJsonParseError parse_error{};
    QJsonDocument doc = QJsonDocument::fromJson(response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning()
            << "Error while parsing JSON response from latest mod version at "
            << parse_error.offset << " reason: " << parse_error.errorString();
        qWarning() << response;
        return ver;
    }

    try
    {
        auto obj = Json::requireObject(doc);
        auto arr = Json::requireArray(obj, "data");

        QJsonObject latest_file_obj;
        ModPlatform::IndexedVersion ver_tmp;

        for (auto file : std::as_const(arr))
        {
            auto file_obj = Json::requireObject(file);
            auto file_tmp = FlameMod::loadIndexedPackVersion(file_obj);
            if (file_tmp.date > ver_tmp.date)
            {
                ver_tmp = file_tmp;
                latest_file_obj = file_obj;
            }
        }

        ver = FlameMod::loadIndexedPackVersion(latest_file_obj);
    }
    catch (Json::JsonException& e)
    {
        qCritical() << "Failed to parse response from a version request.";
        qCritical() << e.what();
        qDebug() << doc;
    }

    return ver;
}

ModPlatform::IndexedVersion FlameAPI::getLatestVersion(VersionSearchArgs&& args)
{
    QEventLoop loop;

    auto netJob =
        new NetJob(QString("Flame::GetLatestVersion(%1)").arg(args.addonId),
                   APPLICATION->network());
    auto response = new QByteArray();
    ModPlatform::IndexedVersion ver;

    netJob->addNetAction(
        Net::Download::makeByteArray(getVersionsURL(args), response));

    QObject::connect(netJob, &NetJob::succeeded,
                     std::bind(storeLatestVersion, response, std::ref(ver)));

    QObject::connect(
        netJob, &NetJob::finished,
        std::bind(finishBlockingJob, netJob, response, std::ref(loop)));

    netJob->start();

    loop.exec();

    return ver;
}

NetJob* FlameAPI::getProjects(QStringList addonIds, QByteArray* response) const
{
    auto* netJob =
        new NetJob(QString("Flame::GetProjects"), APPLICATION->network());

    QJsonObject body_obj;
    QJsonArray addons_arr;
    for (auto& addonId : addonIds)
    {
        addons_arr.append(addonId);
    }

    body_obj["modIds"] = addons_arr;

    QJsonDocument body(body_obj);
    auto body_raw = body.toJson();

    netJob->addNetAction(Net::Upload::makeByteArray(
        QString("https://api.curseforge.com/v1/mods"), response, body_raw));

    QObject::connect(netJob, &NetJob::finished,
                     std::bind(releaseJob, response, netJob));
    QObject::connect(netJob, &NetJob::failed, std::bind(logBody, body_raw));

    return netJob;
}

NetJob* FlameAPI::getFiles(const QStringList& fileIds,
                           QByteArray* response) const
{
    auto* netJob =
        new NetJob(QString("Flame::GetFiles"), APPLICATION->network());

    QJsonObject body_obj;
    QJsonArray files_arr;
    for (auto& fileId : fileIds)
    {
        files_arr.append(fileId);
    }

    body_obj["fileIds"] = files_arr;

    QJsonDocument body(body_obj);
    auto body_raw = body.toJson();

    netJob->addNetAction(Net::Upload::makeByteArray(
        QString("https://api.curseforge.com/v1/mods/files"), response,
        body_raw));

    QObject::connect(netJob, &NetJob::finished,
                     std::bind(releaseJob, response, netJob));
    QObject::connect(netJob, &NetJob::failed, std::bind(logBody, body_raw));

    return netJob;
}
