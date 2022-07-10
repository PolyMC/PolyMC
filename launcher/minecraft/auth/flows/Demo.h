#pragma once
#include "AuthFlow.h"

class DemoRefresh : public AuthFlow
{
    Q_OBJECT
   public:
    explicit DemoRefresh(
        AccountData *data,
        QObject *parent = 0
    );
};

class DemoLogin : public AuthFlow
{
    Q_OBJECT
   public:
    explicit DemoLogin(
        AccountData *data,
        QObject *parent = 0
    );
};