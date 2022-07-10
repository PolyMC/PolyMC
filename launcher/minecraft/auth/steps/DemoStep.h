#pragma once
#include <QObject>

#include "QObjectPtr.h"
#include "minecraft/auth/AuthStep.h"

#include <katabasis/DeviceFlow.h>

class DemoStep : public AuthStep {
    Q_OBJECT
   public:
    explicit DemoStep(AccountData *data);
    virtual ~DemoStep() noexcept;

    void perform() override;
    void rehydrate() override;

    QString describe() override;
};