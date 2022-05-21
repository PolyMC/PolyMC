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
    bool failed = false;
    int index = 0;
    auto doc = Json::requireDocument(*result);
    auto array = doc.object()["data"].toArray();
    for (QJsonValueRef file : array) {
        auto& out = m_toProcess.files[file.toObject()["id"].toInt()];
        try {
            bool fail = (!out.parseFromObject(file.toObject()));
            if(fail){
                //failed :( probably disabled mod, try to add to the list
                if (!doc.isObject()) {
                    throw JSONValidationError(QString("data is not an object? that's not supposed to happen"));
                }
                auto obj = Json::ensureObject(doc.object(), "data");
                //FIXME : HACK, MAY NOT WORK FOR LONG
                out.url = QUrl(QString("https://media.forgecdn.net/files/%1/%2/%3")
                        .arg(QString::number(QString::number(out.fileId).leftRef(4).toInt())
                             ,QString::number(QString::number(out.fileId).rightRef(3).toInt())
                             ,QUrl::toPercentEncoding(out.fileName)), QUrl::TolerantMode);
            }
            failed &= fail;
        } catch (const JSONValidationError& e) {
            qCritical() << "Resolving failed because of a parsing error:";
            qCritical() << e.cause();
            qCritical() << "JSON:";
            qCritical() << file;
            failed = true;
        }
        index++;
    }
    if (!failed) {
        emitSucceeded();
    } else {
        emitFailed(tr("Some mod ID resolving tasks failed."));
    }
}
