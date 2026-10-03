#pragma once

#include "modplatform/ModAPI.h"

class NetworkModAPI : public ModAPI {
   public:
    void searchMods(CallerType* caller, SearchArgs&& args) const override;
    void getModInfo(ModPlatform::IndexedPack& pack, std::function<void(QJsonDocument&, ModPlatform::IndexedPack&)> callback) override;
    void getVersions(VersionSearchArgs&& args, std::function<void(QJsonDocument&, QString)> callback) const override;

    NetJob* getProject(QString addonId, QByteArray* response) const override;

   protected:
    virtual QString getModSearchURL(SearchArgs& args) const = 0;
    virtual QString getModInfoURL(QString& id) const = 0;
    virtual QString getVersionsURL(VersionSearchArgs& args) const = 0;
};
