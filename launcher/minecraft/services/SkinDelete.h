#pragma once

#include <QString>
#include <QNetworkReply>
#include "tasks/Task.h"
#include "QObjectPtr.h"

typedef shared_qobject_ptr<class SkinDelete> SkinDeletePtr;

class SkinDelete : public Task
{
    Q_OBJECT
public:
    SkinDelete(QObject* parent, QString token);
    virtual ~SkinDelete() = default;

private:
    QString m_token;
    shared_qobject_ptr<QNetworkReply> m_reply;

protected:
    virtual void executeTask();

public slots:
    void downloadError(QNetworkReply::NetworkError);
    void downloadFinished();
};
