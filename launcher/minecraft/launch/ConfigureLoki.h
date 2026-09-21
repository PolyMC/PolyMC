#pragma once

#include <launch/LaunchStep.h>
#include <minecraft/auth/MinecraftAccount.h>
#include "net/NetJob.h"

class ConfigureLoki : public LaunchStep {
    Q_OBJECT
   public:
    explicit ConfigureLoki(LaunchTask* parent, QString yggdrasil_base_url, std::shared_ptr<QString> javaagent_arg);
    virtual ~ConfigureLoki() {};

    void executeTask() override;
    void finalize() override;
    bool canAbort() const override { return false; }

   private:
    std::unique_ptr<NetJob> m_job;
    std::shared_ptr<QString> m_javaagent_arg;
    QString m_yggdrasil_base_url;
};
