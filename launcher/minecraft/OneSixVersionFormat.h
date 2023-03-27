#pragma once

#include <minecraft/VersionFile.h>
#include <minecraft/PackProfile.h>
#include <minecraft/Library.h>
#include <ProblemProvider.h>

class OneSixVersionFormat
{
public:
    // version files / profile patches
    static VersionFilePtr versionFileFromJson(const nlohmann::json& doc, const QString& filename, const bool requireOrder);
    static nlohmann::json versionFileToJson(const VersionFilePtr& patch);

    // libraries
    static LibraryPtr libraryFromJson(ProblemContainer& problems, const nlohmann::json& libObj, const QString& filename);
    static nlohmann::json libraryToJson(Library* library);

    // DEPRECATED: old 'plus' jar mods generated by the application
    static LibraryPtr plusJarModFromJson(ProblemContainer& problems, const nlohmann::json& libObj, const QString& filename, const QString& originalName);

    // new jar mods derived from libraries
    static LibraryPtr jarModFromJson(ProblemContainer& problems, const nlohmann::json& libObj, const QString& filename);
    static nlohmann::json jarModtoJson(Library* jarmod);

    // mods, also derived from libraries
    static LibraryPtr modFromJson(ProblemContainer& problems, const nlohmann::json& libObj, const QString& filename);
    static nlohmann::json modtoJson(Library* jarmod);
};
