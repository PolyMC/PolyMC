#pragma once

#include "tasks/Task.h"
#include <QFile>
#include <QtNetwork/QtNetwork>

using SkinDeletePtr = class SkinDelete;

class SkinDelete : public Task
{
    Q_OBJECT
public:
    SkinDelete(QObject *parent, QString token);
    ~SkinDelete() override = default;

private:
    QString m_token;
    shared_qobject_ptr<QNetworkReply> m_reply;

protected:
    void executeTask() override;

public slots:
    void downloadError(QNetworkReply::NetworkError);
    void downloadFinished();
};
