#pragma once

#include <fstream>

#include <minecraft/VersionFile.h>
#include <minecraft/Library.h>
#include <QJsonDocument>
#include <ProblemProvider.h>
#include "json.hpp"

class MojangVersionFormat
{
friend class OneSixVersionFormat;
protected:
    // does not include libraries
    static void readVersionProperties(const nlohmann::json& in, VersionFile* out);
    // does not include libraries
    static void writeVersionProperties(const VersionFile* in, nlohmann::json& out);
public:
    // version files / profile patches
    static VersionFilePtr versionFileFromJson(const nlohmann::json& doc, const QString& filename);
    static nlohmann::json versionFileToJson(const VersionFilePtr& patch);

    // libraries
    static LibraryPtr libraryFromJson(ProblemContainer& problems, const nlohmann::json& libObj, const QString& filename);
    static nlohmann::json libraryToJson(Library* library);
};
