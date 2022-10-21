#pragma once

#include <QFile>
#include <QtNetwork/QtNetwork>
#include <memory>
#include "tasks/Task.h"
#include "QObjectPtr.h"

class CapeChange : public Task
{
    Q_OBJECT
public:
    CapeChange(QObject *parent, const QString &token, const QString &capeId) : Task(parent), m_capeId(capeId), m_token(token) {}
    virtual ~CapeChange() {}

private:
    void setCape(QString & cape);
    void clearCape();

private:
    QString m_capeId;
    QString m_token;
    shared_qobject_ptr<QNetworkReply> m_reply;

protected:
    virtual void executeTask();

public slots:
    void downloadError(QNetworkReply::NetworkError);
    void downloadFinished();
};

