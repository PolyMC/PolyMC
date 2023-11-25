#include "YggdrasilStep.h"

#include "minecraft/auth/AccountData.h"
#include "minecraft/auth/AuthRequest.h"
#include "minecraft/auth/Parsers.h"
#include "minecraft/auth/Yggdrasil.h"

YggdrasilStep::YggdrasilStep(AccountData* data, QString password) : AuthStep(data), m_password(password) {
    m_yggdrasil = new Yggdrasil(m_data, this);

    connect(m_yggdrasil, &Task::failed, this, &YggdrasilStep::onAuthFailed);
    connect(m_yggdrasil, &Task::succeeded, this, &YggdrasilStep::onAuthSucceeded);
    connect(m_yggdrasil, &Task::aborted, this, &YggdrasilStep::onAuthFailed);
}

YggdrasilStep::~YggdrasilStep() noexcept = default;

QString YggdrasilStep::describe() {
  switch(m_data->type) {
    case(AccountType::Mojang):
      return tr("Logging in with Mojang account.");
    case AccountType::AuthlibInjector:
      return tr("Logging in with %1 account.").arg(m_data->authlibInjectorBaseUrl);
    default:
      break;
  }
}

void YggdrasilStep::rehydrate() {
    // NOOP, for now.
}

void YggdrasilStep::perform() {
    if(m_password.size()) {
        m_yggdrasil->login(m_password);
    }
    else {
        m_yggdrasil->refresh();
    }
}

void YggdrasilStep::onAuthSucceeded() {
    emit m_data->type == AccountType::Mojang 
                         ? finished(AccountTaskState::STATE_WORKING, tr("Logged in with Mojang"))
                         : finished(AccountTaskState::STATE_WORKING, tr("Logged in with %1").arg(m_data->authlibInjectorBaseUrl));
}

void YggdrasilStep::onAuthFailed() {
    // TODO: hook these in again, expand to MSA
    // m_error = m_yggdrasil->m_error;
    // m_aborted = m_yggdrasil->m_aborted;

    auto state = m_yggdrasil->taskState();
    QString errorMessage = m_data->type == AccountType::Mojang
                                           ? tr("Mojang user authentication failed.")
                                           : tr("%1 user authentication failed").arg(m_data->authlibInjectorBaseUrl);

    // NOTE: soft error in the first step means 'offline'
    if(state == AccountTaskState::STATE_FAILED_SOFT) {
        state = AccountTaskState::STATE_OFFLINE;
        switch(m_data->type) {
          case AccountType::Mojang:
          {
            errorMessage = tr("Mojang user authentication ended with a network error.");
            break;
          }
          case AccountType::AuthlibInjector:
          {
            if(m_data->authlibInjectorBaseUrl.isEmpty())
            {
              errorMessage = tr("User authentication ended with a network error, did specify a url?");
            } else {
              errorMessage = tr("%1 user authentication ended with a network error").arg(m_data->authlibInjectorBaseUrl);
            }
            break;
          }
          default:
            break;
        }
    }
    emit finished(state, errorMessage);
}
