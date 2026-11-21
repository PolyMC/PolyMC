#pragma once
#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <memory>

#include "minecraft/auth/AuthStep.h"

class AuthlibInjectorStep : public AuthStep
{
    Q_OBJECT

public:
    explicit AuthlibInjectorStep(AccountData* data);
    virtual ~AuthlibInjectorStep() noexcept;

    void perform() override;
    void rehydrate() override;

    QString describe() override;

private slots:
    void onRequestDone();

private:
    std::unique_ptr<QNetworkReply> m_reply;
};
