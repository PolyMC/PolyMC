#pragma once

#include <QByteArray>
#include <QString>

#include "modplatform/ModAPI.h"
#include "net/NetJob.h"

class NetworkModAPI : public ModAPI
{
public:
    NetJob::Ptr searchMods(SearchArgs&& args, QByteArray* response,
                           const QString& caller) const override;
    NetJob::Ptr getVersions(VersionSearchArgs&& args,
                            QByteArray* response) const override;

    NetJob::Ptr getProject(QString addonId,
                           QByteArray* response) const override;

protected:
    virtual QString getModSearchURL(SearchArgs& args) const = 0;
    virtual QString getModInfoURL(QString& id) const = 0;
    virtual QString getVersionsURL(VersionSearchArgs& args) const = 0;
};
