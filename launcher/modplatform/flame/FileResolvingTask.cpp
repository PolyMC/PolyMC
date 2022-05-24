#include "FileResolvingTask.h"
#include "Json.h"
#include "net/Upload.h"

Flame::FileResolvingTask::FileResolvingTask(shared_qobject_ptr<QNetworkAccessManager> network, Flame::Manifest& toProcess)
    : m_network(network), m_toProcess(toProcess)
{}

void Flame::FileResolvingTask::executeTask()
{
    setStatus(tr("Resolving mod IDs..."));
    setProgress(0, m_toProcess.files.size());
    m_dljob = new NetJob("Mod id resolver", m_network);
    result.reset(new QByteArray());
    //build json data to send
    QJsonObject object;

    object["fileIds"] = QJsonArray::fromVariantList(std::accumulate(m_toProcess.files.begin(), m_toProcess.files.end(), QVariantList(), [](QVariantList& l, const File& s) {
        l.push_back(s.fileId);
        return l;
    }));
    QByteArray data = QJsonDocument(object).toJson();
    auto dl = Net::Upload::makeByteArray(QUrl("https://api.curseforge.com/v1/mods/files"), result.get(), data);
    m_dljob->addNetAction(dl);
    connect(m_dljob.get(), &NetJob::finished, this, &Flame::FileResolvingTask::netJobFinished);
    m_dljob->start();
}

void Flame::FileResolvingTask::netJobFinished()
{
    int index = 0;
    // job to check modrinth for blocked projects
    auto job = new NetJob("Modrinth check", m_network);
    blockedProjects = QMap<File *,QByteArray *>();
    auto doc = Json::requireDocument(*result);
    auto array = doc.object()["data"].toArray();
    for (QJsonValueRef file : array) {
        auto& out = m_toProcess.files[file.toObject()["id"].toInt()];
        try {
           out.parseFromObject(file.toObject());
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
