#pragma once

#include <QString>
#include <memory>

#include <launch/LaunchStep.h>
#include "net/HttpMetaCache.h"
#include "net/NetJob.h"

class ConfigureAuthlibInjector : public LaunchStep
{
    Q_OBJECT
public:
    explicit ConfigureAuthlibInjector(LaunchTask* parent,
                                      QString authlibinjector_base_url,
                                      std::shared_ptr<QString> javaagent_arg);
    virtual ~ConfigureAuthlibInjector() {};

    void executeTask() override;
    void finalize() override;
    bool canAbort() const override
    {
        return false;
    }

private:
    void downloadFailed(QString reason);
    void latestJsonDownloaded();
    void javaAgentDownloaded();

    std::unique_ptr<NetJob> m_job;
    MetaEntryPtr m_entry;
    std::shared_ptr<QString> m_javaagent_arg;
    QString m_yggdrasil_base_url;
};
