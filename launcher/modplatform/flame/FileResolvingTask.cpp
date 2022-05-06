#include "FileResolvingTask.h"
#include "Application.h"
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
    bool failed = false;
    int index = 0;
    while (index < results.size()) {
        auto& out = m_toProcess.files[index];
        auto& fileData = results[index];

        try {
            failed &= (!out.parseFromBytes(fileData));
        } catch (const JSONValidationError& e) {
            qCritical() << "Resolving of" << out.projectId << out.fileId << "failed because of a parsing error:";
            qCritical() << e.cause();
            qCritical() << "JSON:";
            qCritical() << fileData;
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
