#include "NetworkModAPI.h"

#include "ui/pages/modplatform/ModModel.h"

#include "Application.h"
#include "net/NetJob.h"

void NetworkModAPI::searchMods(CallerType* caller, SearchArgs&& args) const
{
    auto netJob = new NetJob(QString("%1::Search").arg(caller->debugName()), APPLICATION->network());
    auto searchUrl = getModSearchURL(args);

    auto response = new QByteArray();
    netJob->addNetAction(Net::Download::makeByteArray(QUrl(searchUrl), response));

    QObject::connect(netJob, &NetJob::started, caller, [caller, netJob] { caller->setActiveJob(netJob); });
    QObject::connect(netJob, &NetJob::failed, caller, &CallerType::searchRequestFailed);
    QObject::connect(netJob, &NetJob::succeeded, caller, [caller, response] {
        nlohmann::json doc;
        try
        {
            doc = nlohmann::json::parse(response->constData(), response->constData() + response->size());
        }
        catch (const std::exception& e)
        {
            qWarning() << "Error while parsing JSON response from " << caller->debugName() << " at " << e.what();
            qWarning() << *response;
            return;
        }

        caller->searchRequestFinished(doc);
    });

    netJob->start();
}

void NetworkModAPI::getModInfo(ModPlatform::IndexedPack& pack, std::function<void(nlohmann::json&, ModPlatform::IndexedPack&)> callback)
{
    auto response = new QByteArray();
    auto job = getProject(pack.addonId.toString(), response);

    QObject::connect(job, &NetJob::succeeded, [callback, &pack, response] {
        nlohmann::json doc;
        try
        {
            doc = nlohmann::json::parse(response->constData(), response->constData() + response->size());
        }
        catch (const std::exception& e)
        {
            qWarning() << "Error while parsing JSON response for mod info at " << e.what();
            qWarning() << *response;
            return;
        }

        callback(doc, pack);
    });

    job->start();
}

void NetworkModAPI::getVersions(VersionSearchArgs&& args, std::function<void(nlohmann::json&, QString)> callback) const
{
    auto netJob = new NetJob(QString("ModVersions(%2)").arg(args.addonId), APPLICATION->network());
    auto response = new QByteArray();

    netJob->addNetAction(Net::Download::makeByteArray(getVersionsURL(args), response));

    QObject::connect(netJob, &NetJob::succeeded, [response, callback, args] {
        nlohmann::json doc;
        try
        {
            doc = nlohmann::json::parse(response->constData(), response->constData() + response->size());
        }
        catch (const std::exception& e)
        {
            qWarning() << "Error while parsing JSON response for getting versions at " << e.what();
            qWarning() << *response;
            return;
        }

        callback(doc, args.addonId);
    });

    QObject::connect(netJob, &NetJob::finished, [response, netJob] {
        netJob->deleteLater();
        delete response;
    });

    netJob->start();
}

auto NetworkModAPI::getProject(QString addonId, QByteArray* response) const -> NetJob*
{
    auto netJob = new NetJob(QString("%1::GetProject").arg(addonId), APPLICATION->network());
    auto searchUrl = getModInfoURL(addonId);

    netJob->addNetAction(Net::Download::makeByteArray(QUrl(searchUrl), response));

    QObject::connect(netJob, &NetJob::finished, [response, netJob] {
        netJob->deleteLater();
        delete response;
    });

    return netJob;
}
