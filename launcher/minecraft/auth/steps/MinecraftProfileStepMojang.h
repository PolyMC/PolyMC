#pragma once
#include <QObject>

#include "QObjectPtr.h"
#include "minecraft/auth/AuthStep.h"


class MinecraftProfileStepMojang : public AuthStep {
    Q_OBJECT

public:
    explicit MinecraftProfileStepMojang(AccountData *data);
    virtual ~MinecraftProfileStepMojang() noexcept;

    void perform() override;
    void rehydrate() override;

    QString describe() override;

private slots:
    void onRequestDone(QNetworkReply::NetworkError, QByteArray, QList<QNetworkReply::RawHeaderPair>);

private:
    QString getBaseUrl();
};
