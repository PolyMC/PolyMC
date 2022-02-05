#pragma once

#include "QObjectPtr.h"
#include "tasks/Task.h"
#include <QFile>
#include <QtNetwork/QtNetwork>
#include <memory>

class CapeChange : public Task
{
    Q_OBJECT
public:
    CapeChange(QObject *parent, QString token, QString capeId);
    ~CapeChange() override = default;

private:
    void setCape(QString & cape);
    void clearCape();

private:
    QString m_capeId;
    QString m_token;
    shared_qobject_ptr<QNetworkReply> m_reply;

protected:
    void executeTask() override;

public slots:
    void downloadError(QNetworkReply::NetworkError);
    void downloadFinished();
};

