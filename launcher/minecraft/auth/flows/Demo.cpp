#include "Demo.h"

#include "minecraft/auth/steps/DemoStep.h"

DemoRefresh::DemoRefresh(
    AccountData *data,
    QObject *parent
    ) : AuthFlow(data, parent) {
    m_steps.append(new DemoStep(m_data));
}

DemoLogin::DemoLogin(
    AccountData *data,
    QObject *parent
    ) : AuthFlow(data, parent) {
    m_steps.append(new DemoStep(m_data));
}