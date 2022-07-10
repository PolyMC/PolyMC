#include "DemoStep.h"

#include "Application.h"

DemoStep::DemoStep(AccountData* data) : AuthStep(data) {}
DemoStep::~DemoStep() noexcept = default;

QString DemoStep::describe() {
    return tr("Creating demo account.");
}

void DemoStep::rehydrate() {
    // NOOP
}

void DemoStep::perform() {
    emit finished(AccountTaskState::STATE_WORKING, tr("Created demo account."));
}