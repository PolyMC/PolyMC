#include "Mojang.h"

#include <utility>

#include "minecraft/auth/steps/GetSkinStep.h"
#include "minecraft/auth/steps/MigrationEligibilityStep.h"
#include "minecraft/auth/steps/MinecraftProfileStep.h"
#include "minecraft/auth/steps/YggdrasilStep.h"

MojangRefresh::MojangRefresh(
    AccountData *data,
    QObject *parent
) : AuthFlow(data, parent) {
    m_steps.append(new YggdrasilStep(m_data, QString()));
    m_steps.append(new MinecraftProfileStep(m_data));
    m_steps.append(new MigrationEligibilityStep(m_data));
    m_steps.append(new GetSkinStep(m_data));
}

MojangLogin::MojangLogin(
    AccountData *data,
    QString password,
    QObject *parent
): AuthFlow(data, parent), m_password(std::move(password)) {
    m_steps.append(new YggdrasilStep(m_data, m_password));
    m_steps.append(new MinecraftProfileStep(m_data));
    m_steps.append(new MigrationEligibilityStep(m_data));
    m_steps.append(new GetSkinStep(m_data));
}
