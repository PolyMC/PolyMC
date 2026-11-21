#include "NetworkModAPI.h"

#include "ui/pages/modplatform/ModModel.h"

#include "Application.h"
#include "net/NetJob.h"

#include <functional>

namespace
{

void setCallerJob(ModPlatform::ListModel* caller, NetJob* job)
{
    caller->setActiveJob(job);
}

void handleSearchResponse(ModPlatform::ListModel* caller, QByteArray* response)
{
    QJsonParseError parse_error{};
    QJsonDocument doc = QJsonDocument::fromJson(*response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from "
                   << caller->debugName() << " at " << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << *response;
        return;
    }

    caller->searchRequestFinished(doc);
}

void deleteResponse(QByteArray* response)
{
    delete response;
}

void handleModInfoResponse(
    const std::function<void(QJsonDocument&, ModPlatform::IndexedPack&)>&
        callback,
    ModPlatform::IndexedPack& pack, QByteArray* response)
{
    QJsonParseError parse_error{};
    QJsonDocument doc = QJsonDocument::fromJson(*response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response for mod info at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << *response;
        return;
    }

    callback(doc, pack);
}

void handleVersionsResponse(
    QByteArray* response,
    const std::function<void(QJsonDocument&, QString)>& callback,
    const ModAPI::VersionSearchArgs& args)
{
    QJsonParseError parse_error{};
    QJsonDocument doc = QJsonDocument::fromJson(*response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response for "
                      "getting versions at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << *response;
        return;
    }

    callback(doc, args.addonId);
}

void finishJob(QByteArray* response, NetJob* job)
{
    job->deleteLater();
    delete response;
}

}  // namespace

void NetworkModAPI::searchMods(CallerType* caller, SearchArgs&& args) const
{
    auto netJob = new NetJob(QString("%1::Search").arg(caller->debugName()),
                             APPLICATION->network());
    auto searchUrl = getModSearchURL(args);

    auto response = new QByteArray();
    netJob->addNetAction(
        Net::Download::makeByteArray(QUrl(searchUrl), response));

    QObject::connect(netJob, &NetJob::started, caller,
                     std::bind(setCallerJob, caller, netJob));
    QObject::connect(netJob, &NetJob::failed, caller,
                     &CallerType::searchRequestFailed);
    QObject::connect(netJob, &NetJob::aborted, caller,
                     &CallerType::searchRequestAborted);
    QObject::connect(netJob, &NetJob::succeeded, caller,
                     std::bind(handleSearchResponse, caller, response));
    QObject::connect(netJob, &NetJob::finished,
                     std::bind(deleteResponse, response));

    netJob->start();
}

void NetworkModAPI::getModInfo(
    ModPlatform::IndexedPack& pack,
    std::function<void(QJsonDocument&, ModPlatform::IndexedPack&)> callback)
{
    auto response = new QByteArray();
    auto job = getProject(pack.addonId.toString(), response);

    QObject::connect(
        job, &NetJob::succeeded,
        std::bind(handleModInfoResponse, callback, std::ref(pack), response));

    job->start();
}

void NetworkModAPI::getVersions(
    VersionSearchArgs&& args,
    std::function<void(QJsonDocument&, QString)> callback) const
{
    auto netJob = new NetJob(QString("ModVersions(%2)").arg(args.addonId),
                             APPLICATION->network());
    auto response = new QByteArray();

    netJob->addNetAction(
        Net::Download::makeByteArray(getVersionsURL(args), response));

    QObject::connect(
        netJob, &NetJob::succeeded,
        std::bind(handleVersionsResponse, response, callback, args));

    QObject::connect(netJob, &NetJob::finished,
                     std::bind(finishJob, response, netJob));

    netJob->start();
}

NetJob* NetworkModAPI::getProject(QString addonId, QByteArray* response) const
{
    auto netJob = new NetJob(QString("%1::GetProject").arg(addonId),
                             APPLICATION->network());
    auto searchUrl = getModInfoURL(addonId);

    netJob->addNetAction(
        Net::Download::makeByteArray(QUrl(searchUrl), response));

    QObject::connect(netJob, &NetJob::finished,
                     std::bind(finishJob, response, netJob));

    return netJob;
}
