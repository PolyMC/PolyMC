#include "NetworkModAPI.h"

#include <QUrl>

#include "Application.h"
#include "net/Download.h"

NetJob::Ptr NetworkModAPI::searchMods(SearchArgs&& args, QByteArray* response,
                                      const QString& caller) const
{
    auto netJob =
        new NetJob(QString("%1::Search").arg(caller), APPLICATION->network());
    auto searchUrl = getModSearchURL(args);

    netJob->addNetAction(
        Net::Download::makeByteArray(QUrl(searchUrl), response));

    return netJob;
}

NetJob::Ptr NetworkModAPI::getVersions(VersionSearchArgs&& args,
                                       QByteArray* response) const
{
    auto netJob = new NetJob(QString("ModVersions(%2)").arg(args.addonId),
                             APPLICATION->network());

    netJob->addNetAction(
        Net::Download::makeByteArray(getVersionsURL(args), response));

    return netJob;
}

NetJob::Ptr NetworkModAPI::getProject(QString addonId,
                                      QByteArray* response) const
{
    auto netJob = new NetJob(QString("%1::GetProject").arg(addonId),
                             APPLICATION->network());
    auto searchUrl = getModInfoURL(addonId);

    netJob->addNetAction(
        Net::Download::makeByteArray(QUrl(searchUrl), response));

    return netJob;
}
