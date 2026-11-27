#pragma once

#include <QString>
#include <memory>

#include "ConfigureYggdrasilAgent.h"

class ConfigureLoki : public ConfigureYggdrasilAgent
{
    Q_OBJECT
public:
    explicit ConfigureLoki(LaunchTask* parent, QString server,
                           std::shared_ptr<QString> argument);

protected:
    QString agentName() const override
    {
        return "Loki";
    }
    QString cacheBase() const override
    {
        return "loki";
    }
    QList<Source> sources() const override;
};
