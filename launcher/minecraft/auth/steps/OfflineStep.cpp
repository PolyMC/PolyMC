#include "OfflineStep.h"

#include "Application.h"
#include "minecraft/auth/AccountList.h"
#include "minecraft/auth/AccountTask.h"

OfflineStep::OfflineStep(AccountData* data) : AuthStep(data) {}
OfflineStep::~OfflineStep() noexcept = default;

QString OfflineStep::describe() {
    return tr("Creating offline account.");
}

void OfflineStep::rehydrate() {
    // NOOP
}

void OfflineStep::perform() {
    if (!APPLICATION->accounts()->anyAccountIsValid()) {
        emit finished(AccountTaskState::STATE_FAILED_HARD, tr("No valid account available."));
        m_data->minecraftEntitlement.canPlayMinecraft = false;
        m_data->minecraftEntitlement.ownsMinecraft = false;
        m_data->minecraftEntitlement.validity = Katabasis::Validity::None;
        m_data->minecraftProfile.validity = Katabasis::Validity::None;
        return;
    }
    m_data->minecraftEntitlement.canPlayMinecraft = true;
    m_data->minecraftEntitlement.ownsMinecraft = true;
    m_data->minecraftEntitlement.validity = Katabasis::Validity::Certain;
    m_data->minecraftProfile.validity = Katabasis::Validity::Certain;
    emit finished(AccountTaskState::STATE_WORKING, tr("Created offline account."));
}
