#pragma once
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QNetworkReply>

#include "minecraft/auth/AuthStep.h"

#include <katabasis/Bits.h>

class XboxAuthorizationStep : public AuthStep
{
    Q_OBJECT

public:
    explicit XboxAuthorizationStep(AccountData* data, Katabasis::Token* token,
                                   QString relyingParty,
                                   QString authorizationKind);
    virtual ~XboxAuthorizationStep() noexcept;

    void perform() override;
    void rehydrate() override;

    QString describe() override;

private:
    bool processSTSError(QNetworkReply::NetworkError error, QByteArray data,
                         QList<QNetworkReply::RawHeaderPair> headers);

private slots:
    void onRequestDone(QNetworkReply::NetworkError, QByteArray,
                       QList<QNetworkReply::RawHeaderPair>);

private:
    Katabasis::Token* m_token;
    QString m_relyingParty;
    QString m_authorizationKind;
};
