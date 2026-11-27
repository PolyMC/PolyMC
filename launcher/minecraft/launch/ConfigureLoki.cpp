#include "ConfigureLoki.h"

#include <QUrl>
#include <utility>

ConfigureLoki::ConfigureLoki(LaunchTask* parent, QString server,
                             std::shared_ptr<QString> argument)
    : ConfigureYggdrasilAgent(parent, std::move(server), std::move(argument))
{
}

QList<ConfigureYggdrasilAgent::Source> ConfigureLoki::sources() const
{
    QList<Source> list;
    // A metadata server the user set, like an offline mirror, comes first
    auto meta = metaUrlOverride();
    if (!meta.isEmpty())
    {
        list.append(
            {true, meta.resolved(QUrl("org.unmojang.loki/")), "override/"});
    }
    list.append(
        {true, QUrl("https://meta.unmojang.org/v1/org.unmojang.loki/"), ""});
    return list;
}

#include "moc_ConfigureLoki.cpp"
