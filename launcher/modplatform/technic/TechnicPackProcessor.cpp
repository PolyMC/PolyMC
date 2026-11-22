/* Copyright 2020-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TechnicPackProcessor.h"

#include <FileSystem.h>
#include <Json.h>
#include <minecraft/MinecraftInstance.h>
#include <minecraft/PackProfile.h>
#include <quazip/quazip.h>
#include <quazip/quazipdir.h>
#include <quazip/quazipfile.h>
#include <settings/INIFile.h>
#include <settings/INISettingsObject.h>

#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>

#include <memory>

void Technic::TechnicPackProcessor::run(SettingsObjectPtr globalSettings,
                                        const QString& instName,
                                        const QString& instIcon,
                                        const QString& stagingPath,
                                        const QString& minecraftVersion,
                                        const bool /*isSolder*/)
{
    QString minecraftPath = FS::PathCombine(stagingPath, ".minecraft");
    QString configPath = FS::PathCombine(stagingPath, "instance.cfg");
    auto instanceSettings = std::make_shared<INISettingsObject>(configPath);
    MinecraftInstance instance(globalSettings, instanceSettings, stagingPath);

    instance.setName(instName);

    if (instIcon != "default")
    {
        instance.setIconKey(instIcon);
    }

    auto components = instance.getPackProfile();
    components->buildingFromScratch();

    QByteArray data;

    QString modpackJar = FS::PathCombine(minecraftPath, "bin", "modpack.jar");
    QString versionJson = FS::PathCombine(minecraftPath, "bin", "version.json");
    QString fmlMinecraftVersion;
    QuaZip zipFile(modpackJar);
    QString error_message;
    bool processed = false;
    do
    {
        if (QFile::exists(modpackJar))
        {
            if (!zipFile.open(QuaZip::mdUnzip))
            {
                error_message = tr("Unable to open \"bin/modpack.jar\" file!");
                break;
            }
            QuaZipDir zipFileRoot(&zipFile, "/");
            if (!zipFileRoot.exists("/version.json"))
            {
                if (minecraftVersion.isEmpty())
                {
                    error_message =
                        tr("Could not find \"version.json\" inside "
                           "\"bin/modpack.jar\", but Minecraft version is "
                           "unknown");
                    break;
                }
                components->setComponentVersion("net.minecraft",
                                                minecraftVersion, true);
                components->installJarMods({modpackJar});

                // Forge for 1.4.7 and for 1.5.2 require extra libraries.
                // Figure out the forge version and add it as a component
                // (the code still comes from the jar mod installed above)
                if (zipFileRoot.exists("/forgeversion.properties"))
                {
                    zipFile.setCurrentFile("forgeversion.properties",
                                           QuaZip::csSensitive);
                    QuaZipFile file(&zipFile);
                    if (!file.open(QIODevice::ReadOnly))
                    {
                        // Really shouldn't happen, but error handling shall not
                        // be forgotten
                        error_message =
                            tr("Unable to open \"forgeversion.properties\"");
                        break;
                    }
                    QByteArray forgeVersionData = file.readAll();
                    file.close();
                    INIFile iniFile;
                    iniFile.loadFile(forgeVersionData);
                    QString major, minor, revision, build;
                    major = iniFile["forge.major.number"].toString();
                    minor = iniFile["forge.minor.number"].toString();
                    revision = iniFile["forge.revision.number"].toString();
                    build = iniFile["forge.build.number"].toString();

                    if (major.isEmpty() || minor.isEmpty() ||
                        revision.isEmpty() || build.isEmpty())
                    {
                        error_message =
                            tr("Invalid \"forgeversion.properties\"!");
                        break;
                    }

                    components->setComponentVersion(
                        "net.minecraftforge",
                        major + '.' + minor + '.' + revision + '.' + build);
                }

                processed = true;
                break;
            }
            if (zipFileRoot.exists("/fmlversion.properties"))
            {
                zipFile.setCurrentFile("fmlversion.properties");
                QuaZipFile file(&zipFile);
                if (!file.open(QIODevice::ReadOnly))
                {
                    error_message =
                        tr("Unable to open \"fmlversion.properties\"!");
                    break;
                }
                QByteArray fmlVersionData = file.readAll();
                file.close();
                INIFile iniFile;
                iniFile.loadFile(fmlVersionData);
                // If not present, this evaluates to a null string
                fmlMinecraftVersion = iniFile["fmlbuild.mcversion"].toString();
            }
            zipFile.setCurrentFile("version.json", QuaZip::csSensitive);
            QuaZipFile file(&zipFile);
            if (!file.open(QIODevice::ReadOnly))
            {
                error_message = tr("Unable to open \"version.json\"!");
                break;
            }
            data = file.readAll();
            file.close();
        }
        else if (QFile::exists(versionJson))
        {
            QFile file(versionJson);
            if (!file.open(QIODevice::ReadOnly))
            {
                error_message = tr("Unable to open \"version.json\"!");
                break;
            }
            data = file.readAll();
            file.close();
        }
        else
        {
            // This is the "Vanilla" modpack, excluded by the search code
            error_message = tr("Unable to find a \"version.json\"!");
            break;
        }

        try
        {
            QJsonDocument doc = Json::requireDocument(data);
            QJsonObject root = Json::requireObject(doc, "version.json");
            QString jsonMinecraftVersion =
                Json::ensureString(root, "inheritsFrom", QString(), "");
            if (jsonMinecraftVersion.isEmpty())
            {
                if (fmlMinecraftVersion.isEmpty())
                {
                    error_message = tr(
                        "Could not understand \"version.json\":\ninheritsFrom "
                        "is missing");
                    break;
                }
                jsonMinecraftVersion = fmlMinecraftVersion;
            }
            components->setComponentVersion("net.minecraft",
                                            jsonMinecraftVersion, true);
            for (auto library : Json::ensureArray(root, "libraries", {}))
            {
                if (!library.isObject())
                {
                    continue;
                }

                auto libraryObject = Json::ensureObject(library, {}, "");
                auto libraryName =
                    Json::ensureString(libraryObject, "name", "", "");

                if (libraryName.startsWith("net.minecraftforge:forge:") &&
                    libraryName.contains('-'))
                {
                    QString libraryVersion = libraryName.section(':', 2);
                    if (!libraryVersion.startsWith("1.7.10-"))
                    {
                        components->setComponentVersion(
                            "net.minecraftforge", libraryName.section('-', 1));
                    }
                    else
                    {
                        // 1.7.10 versions sometimes look
                        // like 1.7.10-10.13.4.1614-1.7.10, this filters out
                        // the 10.13.4.1614 part
                        components->setComponentVersion(
                            "net.minecraftforge",
                            libraryName.section('-', 1, 1));
                    }
                }
                else
                {
                    // <Technic library name prefix> -> <our component name>
                    static QMap<QString, QString> loaderMap{
                        {"net.minecraftforge:minecraftforge:",
                         "net.minecraftforge"},
                        {"net.fabricmc:fabric-loader:",
                         "net.fabricmc.fabric-loader"},
                        {"org.quiltmc:quilt-loader:",
                         "org.quiltmc.quilt-loader"}};
                    for (const auto& loader : loaderMap.keys())
                    {
                        if (libraryName.startsWith(loader))
                        {
                            components->setComponentVersion(
                                loaderMap.value(loader),
                                libraryName.section(':', 2));
                            break;
                        }
                    }
                }
            }
        }
        catch (const JSONValidationError& e)
        {
            error_message =
                tr("Could not understand \"version.json\":\n") + e.cause();
            break;
        }

        processed = true;
    } while (0);

    zipFile.close();

    if (!processed)
    {
        emit failed(error_message);
        return;
    }

    components->saveNow();
    emit succeeded();
}
