#pragma once

#include "tasks/Task.h"
#include <QFile>
#include <QtNetwork/QtNetwork>
#include <memory>

typedef shared_qobject_ptr<class SkinUpload> SkinUploadPtr;

class SkinUpload : public Task
{
    Q_OBJECT
public:
    enum Model
    {
        STEVE,
        ALEX
    };

    // Note this class takes ownership of the file.
    SkinUpload(QObject *parent, QString token, QByteArray skin, Model model = STEVE);
    ~SkinUpload() override = default;

private:
    Model m_model;
    QByteArray m_skin;
    QString m_token;
    shared_qobject_ptr<QNetworkReply> m_reply;
protected:
    void executeTask() override;

public slots:

    void downloadError(QNetworkReply::NetworkError);

    void downloadFinished();
};
