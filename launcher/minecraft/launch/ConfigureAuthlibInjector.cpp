#include "ConfigureAuthlibInjector.h"

#include <QUrl>
#include <utility>

ConfigureAuthlibInjector::ConfigureAuthlibInjector(
    LaunchTask* parent, QString server, std::shared_ptr<QString> argument)
    : ConfigureYggdrasilAgent(parent, std::move(server), std::move(argument))
{
}

QList<ConfigureYggdrasilAgent::Source> ConfigureAuthlibInjector::sources() const
{
    QList<Source> list;
    // A metadata server the user set, like an offline mirror, comes first. It
    // lists authlib-injector as a component, like Fjord Launcher's does.
    auto meta = metaUrlOverride();
    if (!meta.isEmpty())
    {
        list.append({true, meta.resolved(QUrl("moe.yushi.authlibinjector/")),
                     "override/"});
    }
    list.append(
        {false, QUrl("https://authlib-injector.yushi.moe/artifact/latest.json"),
         ""});
    return list;
}

#include "moc_ConfigureAuthlibInjector.cpp"
