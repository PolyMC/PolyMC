#pragma once

#include <launch/LaunchStep.h>
#include <minecraft/auth/MinecraftAccount.h>
#include "net/NetJob.h"

class ConfigureAuthlibInjector: public LaunchStep
{
    Q_OBJECT
public:
    explicit ConfigureAuthlibInjector(LaunchTask *parent, QString authlibinjector_base_url, std::shared_ptr<QString> javaagent_arg);
    virtual ~ConfigureAuthlibInjector() {};

    void executeTask() override;
    void finalize() override;
    bool canAbort() const override
    {
        return false;
    }
private:
    std::unique_ptr<NetJob> m_job;
    std::shared_ptr<QString> m_javaagent_arg;
    QString m_authlibinjector_base_url;
};
