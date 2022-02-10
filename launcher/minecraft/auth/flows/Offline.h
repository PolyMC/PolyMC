#pragma once
#include "AuthFlow.h"

class OfflineRefresh : public AuthFlow
{
    Q_OBJECT
public:
    explicit OfflineRefresh(
        AccountData *data,
        QObject *parent = nullptr
    );
};

class OfflineLogin : public AuthFlow
{
    Q_OBJECT
public:
    explicit OfflineLogin(
        AccountData *data,
        QObject *parent = nullptr
    );
};
