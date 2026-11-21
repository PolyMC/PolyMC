#pragma once
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QNetworkReply>

#include "minecraft/auth/AuthStep.h"

class MigrationEligibilityStep : public AuthStep
{
    Q_OBJECT

public:
    explicit MigrationEligibilityStep(AccountData* data);
    virtual ~MigrationEligibilityStep() noexcept;

    void perform() override;
    void rehydrate() override;

    QString describe() override;

private slots:
    void onRequestDone(QNetworkReply::NetworkError, QByteArray,
                       QList<QNetworkReply::RawHeaderPair>);
};
