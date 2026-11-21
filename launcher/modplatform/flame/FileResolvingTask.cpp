#include "FileResolvingTask.h"

#include "Json.h"
#include "net/Download.h"
#include "net/Upload.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUrl>
#include <QVariantList>

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <utility>

namespace
{

QVariantList appendFileId(QVariantList& list, const Flame::File& file)
{
    list.push_back(file.fileId);
    return list;
}

void markResolved(Flame::File& file)
{
    file.resolved = true;
}

bool isUnresolved(Flame::File* file)
{
    return !file->resolved;
}

}  // namespace

Flame::FileResolvingTask::FileResolvingTask(
    const shared_qobject_ptr<QNetworkAccessManager>& network,
    Flame::Manifest& toProcess)
    : m_network(network), m_toProcess(toProcess)
{
}

Flame::FileResolvingTask::~FileResolvingTask()
{
    if (m_dljob && m_dljob->isRunning())
    {
        disconnect(m_dljob.get(), nullptr, this, nullptr);
        m_dljob->abort();
    }
}

bool Flame::FileResolvingTask::abort()
{
    if (m_dljob)
    {
        return m_dljob->abort();
    }
    return true;
}

void Flame::FileResolvingTask::executeTask()
{
    setStatus(tr("Resolving mod IDs..."));
    setProgress(0, 3);
    m_dljob = new NetJob("Mod id resolver", m_network);
    result.reset(new QByteArray());
    // build json data to send
    QJsonObject object;

    object["fileIds"] = QJsonArray::fromVariantList(
        std::accumulate(m_toProcess.files.begin(), m_toProcess.files.end(),
                        QVariantList(), appendFileId));
    QByteArray data = Json::toText(object);
    auto dl = Net::Upload::makeByteArray(
        QUrl("https://api.curseforge.com/v1/mods/files"), result.get(), data);
    m_dljob->addNetAction(dl);
    connect(m_dljob.get(), &NetJob::finished, this,
            &Flame::FileResolvingTask::netJobFinished);
    m_dljob->start();
}

bool Flame::FileResolvingTask::stageAborted()
{
    if (!isRunning())
    {
        return true;
    }
    if (m_dljob->getState() != Task::State::AbortedByUser)
    {
        return false;
    }
    qDeleteAll(blockedProjects);
    blockedProjects.clear();
    emitAborted();
    return true;
}

void Flame::FileResolvingTask::netJobFinished()
{
    if (stageAborted())
    {
        return;
    }
    setProgress(1, 3);
    blockedProjects = QMap<File*, QByteArray*>();
    QJsonDocument doc;

    try
    {
        doc = Json::requireDocument(*result);
    }
    catch (const JSONValidationError& e)
    {
        qDebug() << "Flame::FileResolvingTask: Json Validation error: "
                 << e.what();
        emitFailed(e.what());
        return;
    }

    // This is a slot, an exception escaping from here would take the whole
    // launcher down
    QJsonArray array;
    try
    {
        array = Json::requireArray(doc.object()["data"]);
        for (const auto& file : std::as_const(array))
        {
            Json::requireInteger(Json::requireObject(file)["id"]);
        }
    }
    catch (const JSONValidationError& e)
    {
        qDebug() << "Flame::FileResolvingTask: Json Validation error: "
                 << e.what();
        emitFailed(e.what());
        return;
    }

    // job to check modrinth for blocked projects
    m_dljob = new NetJob("Modrinth check", m_network);
    for (const auto& file : std::as_const(array))
    {
        auto fileid = Json::requireInteger(Json::requireObject(file)["id"]);
        auto& out = m_toProcess.files[fileid];
        try
        {
            out.parseFromObject(Json::requireObject(file));
        }
        catch (const JSONValidationError&)
        {
            qDebug() << "Blocked mod on curseforge" << out.fileName;
            auto hash = out.hash;
            if (!hash.isEmpty())
            {
                auto url = QString(
                               "https://api.modrinth.com/v2/version_file/"
                               "%1?algorithm=sha1")
                               .arg(hash);
                auto output = new QByteArray();
                auto dl = Net::Download::makeByteArray(QUrl(url), output);
                QObject::connect(dl.get(), &Net::Download::succeeded,
                                 std::bind(markResolved, std::ref(out)));

                m_dljob->addNetAction(dl);
                blockedProjects.insert(&out, output);
            }
        }
    }
    connect(m_dljob.get(), &NetJob::finished, this,
            &Flame::FileResolvingTask::modrinthCheckFinished);

    m_dljob->start();
}

void Flame::FileResolvingTask::modrinthCheckFinished()
{
    if (stageAborted())
    {
        return;
    }
    setProgress(2, 3);
    qDebug() << "Finished with blocked mods : " << blockedProjects.size();

    for (auto it = blockedProjects.keyBegin(); it != blockedProjects.keyEnd();
         it++)
    {
        auto& out = *it;
        auto bytes = blockedProjects[out];
        if (!out->resolved)
        {
            delete bytes;
            continue;
        }
        QJsonDocument doc = QJsonDocument::fromJson(*bytes);
        auto obj = doc.object();
        try
        {
            auto array = Json::requireArray(obj, "files");
            for (auto file : std::as_const(array))
            {
                auto fileObj = Json::requireObject(file);
                auto primary = Json::requireBoolean(fileObj, "primary");
                if (primary)
                {
                    out->url = Json::requireUrl(fileObj, "url");
                    qDebug()
                        << "Found alternative on modrinth " << out->fileName;
                    break;
                }
            }
        }
        catch (const JSONValidationError& e)
        {
            // Treat it like it wasn't found on Modrinth, so the user gets asked
            // to download it
            qWarning() << "Couldn't read the Modrinth alternative of"
                       << out->fileName << ":" << e.cause();
            out->resolved = false;
        }
        delete bytes;
    }
    // copy to an output list and filter out projects found on modrinth
    m_blocked.clear();
    auto it = blockedProjects.keys();
    std::copy_if(it.begin(), it.end(), std::back_inserter(m_blocked),
                 isUnresolved);
    blockedProjects.clear();
    // Display not found mods early
    if (!m_blocked.empty())
    {
        // blocked mods found, we need the slug for displaying.... we need
        // another job :D !
        m_slugs = QVector<QByteArray>(m_blocked.size());
        m_dljob = new NetJob("Slug Job", m_network);
        auto index = 0;
        for (auto fileInfo : std::as_const(m_blocked))
        {
            auto projectId = fileInfo->projectId;
            auto url =
                QString("https://api.curseforge.com/v1/mods/%1").arg(projectId);
            auto dl = Net::Download::makeByteArray(url, &m_slugs[index]);
            m_dljob->addNetAction(dl);
            index++;
        }
        // Use finished: the links are only used for display, one failed lookup
        // shouldn't stop everything
        connect(m_dljob.get(), &NetJob::finished, this,
                &Flame::FileResolvingTask::onSlugJobFinished);
        m_dljob->start();
    }
    else
    {
        emitSucceeded();
    }
}

void Flame::FileResolvingTask::onSlugJobFinished()
{
    if (stageAborted())
    {
        return;
    }
    auto index = 0;
    for (const auto& slugResult : std::as_const(m_slugs))
    {
        auto mod = m_blocked.at(index);
        index++;
        try
        {
            auto json = QJsonDocument::fromJson(slugResult);
            auto base = Json::requireString(
                Json::requireObject(
                    Json::requireObject(Json::requireObject(json), "data"),
                    "links"),
                "websiteUrl");
            auto link = QString("%1/download/%2")
                            .arg(base, QString::number(mod->fileId));
            mod->websiteUrl = link;
        }
        catch (const JSONValidationError& e)
        {
            qWarning() << "Couldn't get the website of blocked mod"
                       << mod->fileName << ":" << e.cause();
        }
    }
    emitSucceeded();
}
