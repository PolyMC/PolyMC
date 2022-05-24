#include "FileResolvingTask.h"
#include "Json.h"

Flame::FileResolvingTask::FileResolvingTask(shared_qobject_ptr<QNetworkAccessManager> network, Flame::Manifest& toProcess)
    : m_network(network), m_toProcess(toProcess)
{}

void Flame::FileResolvingTask::executeTask()
{
    setStatus(tr("Resolving mod IDs..."));
    setProgress(0, m_toProcess.files.size());
    m_dljob = new NetJob("Mod id resolver", m_network);
    results.resize(m_toProcess.files.size());
    int index = 0;
    for (auto& file : m_toProcess.files) {
        auto projectIdStr = QString::number(file.projectId);
        auto fileIdStr = QString::number(file.fileId);
        QString metaurl = QString("https://api.curseforge.com/v1/mods/%1/files/%2").arg(projectIdStr, fileIdStr);
        auto dl = Net::Download::makeByteArray(QUrl(metaurl), &results[index]);
        m_dljob->addNetAction(dl);
        index++;
    }
    connect(m_dljob.get(), &NetJob::finished, this, &Flame::FileResolvingTask::netJobFinished);
    m_dljob->start();
}

void Flame::FileResolvingTask::netJobFinished()
{
    int index = 0;
    // job to check modrinth for blocked projects
    auto job = new NetJob("Modrinth check", m_network);
    blockedProjects = QMap<File *,QByteArray *>();
    for (auto& bytes : results) {
        auto& out = m_toProcess.files[index];
        try {
           out.parseFromBytes(bytes);
        } catch (const JSONValidationError& e) {
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
        index++;
    }
    connect(job, &NetJob::finished, this, &Flame::FileResolvingTask::modrinthCheckFinished);

    job->start();
}

void Flame::FileResolvingTask::modrinthCheckFinished() {
    for(auto out : blockedProjects.keys()) {
        auto bytes = blockedProjects[out];
        if(!out->resolved){
            delete bytes;
            continue;
        }
        QJsonDocument doc = QJsonDocument::fromJson(*bytes);
        auto obj = doc.object();
        auto array = obj["files"].toArray();
        for(auto file : array) {
            auto fileObj = file.toObject();
            auto primary = fileObj["primary"].toBool();
            if(primary) {
                out->url = fileObj["url"].toString();
                qDebug() << "Found alternative on modrinth " << out->fileName;
                break;
            }
        }
        delete bytes;
    }
    emitSucceeded();
}
