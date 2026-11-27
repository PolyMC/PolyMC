#include "ModrinthAPI.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringList>
#include <QUrl>

#include "Application.h"
#include "Json.h"
#include "net/Download.h"
#include "net/Upload.h"

NetJob::Ptr ModrinthAPI::currentVersions(const QStringList& hashes,
                                         QString hash_format,
                                         QByteArray* response)
{
    auto* netJob = new NetJob(QString("Modrinth::GetCurrentVersions"),
                              APPLICATION->network());

    QJsonObject body_obj;

    Json::writeStringList(body_obj, "hashes", hashes);
    Json::writeString(body_obj, "algorithm", hash_format);

    QJsonDocument body(body_obj);
    auto body_raw = body.toJson();

    netJob->addNetAction(Net::Upload::makeByteArray(
        apiRoot() + "/version_files", response, body_raw));

    return netJob;
}

NetJob::Ptr ModrinthAPI::latestVersions(const QStringList& hashes,
                                        QString hash_format,
                                        std::list<Version> mcVersions,
                                        ModLoaderTypes loaders,
                                        QByteArray* response)
{
    auto* netJob = new NetJob(QString("Modrinth::GetLatestVersions"),
                              APPLICATION->network());

    QJsonObject body_obj;

    Json::writeStringList(body_obj, "hashes", hashes);
    Json::writeString(body_obj, "algorithm", hash_format);

    Json::writeStringList(body_obj, "loaders", getModLoaderStrings(loaders));

    QStringList game_versions;
    for (auto& ver : mcVersions)
    {
        game_versions.append(ver.toString());
    }
    Json::writeStringList(body_obj, "game_versions", game_versions);

    QJsonDocument body(body_obj);
    auto body_raw = body.toJson();

    netJob->addNetAction(Net::Upload::makeByteArray(
        apiRoot() + "/version_files/update", response, body_raw));

    return netJob;
}

NetJob::Ptr ModrinthAPI::getVersion(const QString& id,
                                    QByteArray* response) const
{
    auto netJob = new NetJob(QString("Modrinth::GetVersion(%1)").arg(id),
                             APPLICATION->network());

    netJob->addNetAction(Net::Download::makeByteArray(
        QUrl(apiRoot() + "/version/" + id), response));

    return netJob;
}

NetJob::Ptr ModrinthAPI::getCurrentUser(QByteArray* response) const
{
    auto netJob =
        new NetJob(QString("Modrinth::GetCurrentUser"), APPLICATION->network());

    netJob->addNetAction(
        Net::Download::makeByteArray(QUrl(apiRoot() + "/user"), response));

    return netJob;
}

NetJob::Ptr ModrinthAPI::getUserProjects(const QString& user,
                                         QByteArray* response) const
{
    auto netJob = new NetJob(QString("Modrinth::GetUserProjects(%1)").arg(user),
                             APPLICATION->network());

    netJob->addNetAction(Net::Download::makeByteArray(
        QUrl(apiRoot() + "/user/" + user + "/projects"), response));

    return netJob;
}

QString ModrinthAPI::projectReference(const QString& text) const
{
    auto trimmed = text.trimmed();
    if (trimmed.isEmpty() || trimmed.contains(' '))
    {
        return {};
    }

    // A link to a project on this source's website or API, like
    // https://modrinth.com/mod/sodium
    QUrl url(trimmed, QUrl::StrictMode);
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty())
    {
        auto host = url.host().toLower();
        if (host != QUrl(websiteRoot()).host().toLower() &&
            host != QUrl(apiRoot()).host().toLower())
        {
            return {};
        }

        static const QStringList project_paths{
            "project",      "mod",    "modpack", "plugin",
            "resourcepack", "shader", "datapack"};
        const auto parts = url.path().split('/', Qt::SkipEmptyParts);
        for (int i = 0; i + 1 < parts.size(); i++)
        {
            if (project_paths.contains(parts.at(i)))
            {
                return parts.at(i + 1);
            }
        }
        return {};
    }

    // Modrinth's rules for slugs, which also fit IDs
    static const QRegularExpression slug(
        QRegularExpression::anchoredPattern(R"([\w!@$()`.+,"\-']{3,64})"));
    return slug.match(trimmed).hasMatch() ? trimmed : QString();
}

NetJob::Ptr ModrinthAPI::getProjects(QStringList addonIds,
                                     QByteArray* response) const
{
    auto netJob =
        new NetJob(QString("Modrinth::GetProjects"), APPLICATION->network());
    auto searchUrl = getMultipleModInfoURL(addonIds);

    netJob->addNetAction(
        Net::Download::makeByteArray(QUrl(searchUrl), response));

    return netJob;
}
