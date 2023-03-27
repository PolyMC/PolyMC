#include "FileResolvingTask.h"

#include <nlohmann/json.hpp>
#include "net/Upload.h"

Flame::FileResolvingTask::FileResolvingTask(const shared_qobject_ptr<QNetworkAccessManager>& network, Flame::Manifest& toProcess)
    : m_network(network), m_toProcess(toProcess)
{}

bool Flame::FileResolvingTask::abort()
{
    if (m_dljob)
        return m_dljob->abort();
    return true;
}

void Flame::FileResolvingTask::executeTask()
{
    setStatus(tr("Resolving mod IDs..."));
    setProgress(0, 3);
    m_dljob = new NetJob("Mod id resolver", m_network);
    result.reset(new QByteArray());
    //build json data to send
    nlohmann::json data;
    data["fileIds"] = std::accumulate(m_toProcess.files.begin(), m_toProcess.files.end(), std::vector<int>(), [](std::vector<int>& l, const File& s) {
        l.push_back(s.fileId);
        return l;
    });

    QString dataString = data.dump().c_str();
    auto dl = Net::Upload::makeByteArray(QUrl("https://api.curseforge.com/v1/mods/files"), result.get(), dataString.toUtf8());
    m_dljob->addNetAction(dl);
    connect(m_dljob.get(), &NetJob::finished, this, &Flame::FileResolvingTask::netJobFinished);
    m_dljob->start();
}

void Flame::FileResolvingTask::netJobFinished()
{
    setProgress(1, 3);
    // job to check modrinth for blocked projects
    auto job = new NetJob("Modrinth check", m_network);
    blockedProjects = QMap<File*,QByteArray*>();
    nlohmann::json doc;

    try {
        doc = nlohmann::json::parse(result->constData(), result->constData() + result->size());
    }
    catch (const nlohmann::json::exception &e) {
        qDebug() << "Flame::FileResolvingTask: Json Validation error: " << e.what();
        emitFailed(e.what());
        return;
    }

    for (const auto& file : doc["data"]) {
        auto fileid = file["id"].get<int>();
        auto& out = m_toProcess.files[fileid];
        try {
           out.parseFromObject(file);
        } catch (const std::exception& e) {
            qDebug() << "Blocked mod on curseforge" << out.fileName;
            auto hash = out.hash;
            if(!hash.isEmpty()) {
                auto url = QString("https://api.modrinth.com/v2/version_file/%1?algorithm=sha1").arg(hash);
                auto output = new QByteArray();
                auto dl = Net::Download::makeByteArray(QUrl(url), output);
                QObject::connect(dl.get(), &Net::Download::succeeded, [&out]() {
                    out.resolved = true;
                });

                job->addNetAction(dl);
                blockedProjects.insert(&out, output);
            }
        }
    }
    connect(job, &NetJob::finished, this, &Flame::FileResolvingTask::modrinthCheckFinished);

    job->start();
}

void Flame::FileResolvingTask::modrinthCheckFinished() {
    setProgress(2, 3);
    qDebug() << "Finished with blocked mods : " << blockedProjects.size();

    for (auto it = blockedProjects.keyBegin(); it != blockedProjects.keyEnd(); it++) {
        auto &out = *it;
        auto bytes = blockedProjects[out];
        if (!out->resolved) {
            delete bytes;
            continue;
        }
        nlohmann::json doc = nlohmann::json::parse(bytes->constData(), bytes->constData() + bytes->size());
        auto array = doc["files"];
        for (auto file: array) {
            auto primary = file["primary"].get<bool>();
            if (primary) {
                out->url = QUrl(file["url"].get<std::string>().c_str(), QUrl::StrictMode);
                qDebug() << "Found alternative on modrinth " << out->fileName;
                break;
            }
        }

        delete bytes;
    }
    //copy to an output list and filter out projects found on modrinth
    auto block = new QList<File *>();
    auto it = blockedProjects.keys();
    std::copy_if(it.begin(), it.end(), std::back_inserter(*block), [](File *f) {
        return !f->resolved;
    });
    //Display not found mods early
    if (!block->empty()) {
        //blocked mods found, we need the slug for displaying.... we need another job :D !
        auto slugJob = new NetJob("Slug Job", m_network);
        auto slugs = QVector<QByteArray>(block->size());
        auto index = 0;
        for (auto fileInfo: *block) {
            auto projectId = fileInfo->projectId;
            slugs[index] = QByteArray();
            auto url = QString("https://api.curseforge.com/v1/mods/%1").arg(projectId);
            auto dl = Net::Download::makeByteArray(url, &slugs[index]);
            slugJob->addNetAction(dl);
            index++;
        }
        connect(slugJob, &NetJob::succeeded, this, [slugs, this, slugJob, block]() {
            slugJob->deleteLater();
            auto index = 0;
            for (const auto &slugResult: slugs) {
                nlohmann::json doc = nlohmann::json::parse(slugResult.constData(), slugResult.constData() + slugResult.size());
                QString base = doc["data"]["links"]["websiteUrl"].get<std::string>().c_str();

                auto mod = block->at(index);
                auto link = QString("%1/download/%2").arg(base, QString::number(mod->fileId));
                mod->websiteUrl = link;
                index++;
            }
            emitSucceeded();
        });
        slugJob->start();
    } else {
        emitSucceeded();
    }
}
