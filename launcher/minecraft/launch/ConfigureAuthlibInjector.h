#pragma once

#include <QString>
#include <memory>

#include "ConfigureYggdrasilAgent.h"

class ConfigureAuthlibInjector : public ConfigureYggdrasilAgent
{
    Q_OBJECT
public:
    explicit ConfigureAuthlibInjector(LaunchTask* parent, QString server,
                                      std::shared_ptr<QString> argument);

protected:
    QString agentName() const override
    {
        return "authlib-injector";
    }
    QString cacheBase() const override
    {
        return "authlibinjector";
    }
    QList<Source> sources() const override;
};
