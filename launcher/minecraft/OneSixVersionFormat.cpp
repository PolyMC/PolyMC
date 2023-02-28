// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "OneSixVersionFormat.h"
#include "minecraft/Agent.h"
#include "minecraft/ParseUtils.h"
#include <minecraft/MojangVersionFormat.h>

static void readString(const nlohmann::json &root, const QString &key, QString &variable)
{
    if (root.contains(key.toStdString()))
    {
        variable = root[key.toStdString()].get<std::string>().c_str();
    }
}

LibraryPtr OneSixVersionFormat::libraryFromJson(ProblemContainer & problems, const nlohmann::json &libObj, const QString &filename)
{
    LibraryPtr out = MojangVersionFormat::libraryFromJson(problems, libObj, filename);
    readString(libObj, "MMC-hint", out->m_hint);
    readString(libObj, "MMC-absulute_url", out->m_absoluteURL);
    readString(libObj, "MMC-absoluteUrl", out->m_absoluteURL);
    readString(libObj, "MMC-filename", out->m_filename);
    readString(libObj, "MMC-displayname", out->m_displayname);
    return out;
}

nlohmann::json OneSixVersionFormat::libraryToJson(Library *library)
{
    nlohmann::json libRoot = MojangVersionFormat::libraryToJson(library);
    if (library->m_absoluteURL.size())
        libRoot["MMC-absoluteUrl"] = library->m_absoluteURL.toStdString();
    if (library->m_hint.size())
            libRoot["MMC-hint"] = library->m_hint.toStdString();
    if (library->m_filename.size())
            libRoot["MMC-filename"] = library->m_filename.toStdString();
    if (library->m_displayname.size())
            libRoot["MMC-displayname"] = library->m_displayname.toStdString();
    return libRoot;

}

VersionFilePtr OneSixVersionFormat::versionFileFromJson(const nlohmann::json &doc, const QString &filename, const bool requireOrder)
{
    VersionFilePtr out(new VersionFile());
    if (doc.empty() || doc.is_null())
    {
        throw std::runtime_error((filename + " is empty or null").toStdString());
    }
    if (!doc.is_object())
    {
        throw std::runtime_error((filename + " is not an object").toStdString());
    }

    Meta::MetadataVersion formatVersion = Meta::parseFormatVersion(doc, false);
    switch(formatVersion)
    {
        case Meta::MetadataVersion::InitialRelease:
            break;
        case Meta::MetadataVersion::Invalid:
            throw std::runtime_error((filename + " has an invalid format version").toStdString());
    }

    if (requireOrder)
    {
        if (doc.contains("order"))
        {
            out->order = doc["order"].get<int>();
        }
        else
        {
            // FIXME: evaluate if we don't want to throw exceptions here instead
            qCritical() << filename << "doesn't contain an order field";
        }
    }

    //out->name = root.value("name").toString();
    out->name = QString::fromStdString(doc["name"]);

    if(doc.contains("uid"))
    {
        out->uid = QString::fromStdString(doc["uid"]);
    }
    else
    {
        out->uid = QString::fromStdString(doc["fileId"]);
    }

    out->version = QString::fromStdString(doc["version"]);

    MojangVersionFormat::readVersionProperties(doc, out.get());

    // added for legacy Minecraft window embedding, TODO: remove
    readString(doc, "appletClass", out->appletClass);

    if (doc.contains("+tweakers"))
    {
        for (auto tweakerVal : doc["+tweakers"])
        {
            out->addTweakers.append(QString::fromStdString(tweakerVal.get<std::string>()));
        }
    }

    if (doc.contains("+traits"))
    {
        for (auto tweakerVal : doc["+traits"])
        {
            out->traits.insert(QString::fromStdString(tweakerVal.get<std::string>()));
        }
    }

    if (doc.contains("+jvmArgs"))
    {
        for (auto arg : doc["+jvmArgs"])
        {
            out->addnJvmArguments.append(QString::fromStdString(arg.get<std::string>()));
        }
    }


    if (doc.contains("jarMods"))
    {
        for (auto libVal : doc["jarMods"])
        {
            nlohmann::json libObj = libVal;
            // parse the jarmod
            auto lib = OneSixVersionFormat::jarModFromJson(*out, libObj, filename);
            // and add to jar mods
            out->jarMods.append(lib);
        }
    }
    else if (doc.contains("+jarMods")) // DEPRECATED: old style '+jarMods' are only here for backwards compatibility
    {
        for (auto libVal : doc["+jarMods"])
        {
            nlohmann::json libObj = libVal;
            // parse the jarmod
            auto lib = OneSixVersionFormat::plusJarModFromJson(*out, libObj, filename, out->name);
            // and add to jar mods
            out->jarMods.append(lib);
        }
    }

    if (doc.contains("mods"))
    {
        for (auto libVal : doc["mods"])
        {
            nlohmann::json libObj = libVal;
            // parse the jarmod
            auto lib = OneSixVersionFormat::modFromJson(*out, libObj, filename);
            // and add to jar mods
            out->mods.append(lib);
        }
    }

    auto readLibs = [&](const char * which, QList<LibraryPtr> & outList)
    {
        for (auto libVal : doc[which])
        {
            nlohmann::json libObj = libVal;
            // parse the library
            auto lib = libraryFromJson(*out, libObj, filename);
            outList.append(lib);
        }
    };
    bool hasPlusLibs = doc.contains("+libraries");
    bool hasLibs = doc.contains("libraries");
    if (hasPlusLibs && hasLibs)
    {
        out->addProblem(ProblemSeverity::Warning,
                        QObject::tr("Version file has both '+libraries' and 'libraries'. This is no longer supported."));
        readLibs("libraries", out->libraries);
        readLibs("+libraries", out->libraries);
    }
    else if (hasLibs)
    {
        readLibs("libraries", out->libraries);
    }
    else if(hasPlusLibs)
    {
        readLibs("+libraries", out->libraries);
    }

    if(doc.contains("mavenFiles")) {
        readLibs("mavenFiles", out->mavenFiles);
    }

    if(doc.contains("+agents")) {
        for (auto agentVal : doc["+agents"])
        {
            nlohmann::json agentObj = agentVal;
            auto lib = libraryFromJson(*out, agentObj, filename);
            QString arg = "";
            if (agentObj.contains("argument"))
            {
                readString(agentObj, "argument", arg);
            }
            AgentPtr agent(new Agent(lib, arg));
            out->agents.append(agent);
        }
    }

    // if we have mainJar, just use it
    if(doc.contains("mainJar"))
    {
        nlohmann::json libObj = doc["mainJar"];
        out->mainJar = libraryFromJson(*out, libObj, filename);
    }
    // else reconstruct it from downloads and id ... if that's available
    else if(!out->minecraftVersion.isEmpty())
    {
        auto lib = std::make_shared<Library>();
        lib->setRawName(GradleSpecifier(QString("com.mojang:minecraft:%1:client").arg(out->minecraftVersion)));
        // we have a reliable client download, use it.
        if(out->mojangDownloads.contains("client"))
        {
            auto LibDLInfo = std::make_shared<MojangLibraryDownloadInfo>();
            LibDLInfo->artifact = out->mojangDownloads["client"];
            lib->setMojangDownloadInfo(LibDLInfo);
        }
        // we got nothing...
        else
        {
            out->addProblem(
                ProblemSeverity::Error,
                QObject::tr("URL for the main jar could not be determined - Mojang removed the server that we used as fallback.")
            );
        }
        out->mainJar = lib;
    }

    if (doc.contains("requires"))
    {
        Meta::parseRequires(doc, &out->requires);
    }

    QString dependsOnMinecraftVersion = QString::fromStdString(doc.value("mcVersion", ""));
    if(!dependsOnMinecraftVersion.isEmpty())
    {
        Meta::Require mcReq;
        mcReq.uid = "net.minecraft";
        mcReq.equalsVersion = dependsOnMinecraftVersion;
        if (out->requires.count(mcReq) == 0)
        {
            out->requires.insert(mcReq);
        }
    }
    if (doc.contains("conflicts"))
    {
        Meta::parseRequires(doc, &out->conflicts);
    }
    if (doc.contains("volatile"))
    {
        out->m_volatile = doc["volatile"];
    }

    /* removed features that shouldn't be used */
    if (doc.contains("tweakers"))
    {
        out->addProblem(ProblemSeverity::Error, QObject::tr("Version file contains unsupported element 'tweakers'"));
    }
    if (doc.contains("-libraries"))
    {
        out->addProblem(ProblemSeverity::Error, QObject::tr("Version file contains unsupported element '-libraries'"));
    }
    if (doc.contains("-tweakers"))
    {
        out->addProblem(ProblemSeverity::Error, QObject::tr("Version file contains unsupported element '-tweakers'"));
    }
    if (doc.contains("-minecraftArguments"))
    {
        out->addProblem(ProblemSeverity::Error, QObject::tr("Version file contains unsupported element '-minecraftArguments'"));
    }
    if (doc.contains("+minecraftArguments"))
    {
        out->addProblem(ProblemSeverity::Error, QObject::tr("Version file contains unsupported element '+minecraftArguments'"));
    }
    return out;
}

nlohmann::json OneSixVersionFormat::versionFileToJson(const VersionFilePtr &patch)
{
    nlohmann::json root;

    root["name"] = patch->name.toStdString();
    root["uid"] = patch->uid.toStdString();
    root["version"] = patch->version.toStdString();

    root["formatVersion"] = int(Meta::MetadataVersion::InitialRelease);

    MojangVersionFormat::writeVersionProperties(patch.get(), root);

    if(patch->mainJar)
    {
        root["mainJar"] = libraryToJson(patch->mainJar.get());
    }

    root["appletClass"] = patch->appletClass.toStdString();
    if (!patch->addTweakers.isEmpty())
    {
        nlohmann::json array;
        for (auto value: patch->addTweakers)
        {
            array.push_back(value.toStdString());
        }

        root["+tweakers"] = array;
    }

    if (!patch->traits.isEmpty())
    {
        nlohmann::json array;
        for (auto value: patch->traits.values())
        {
            array.push_back(value.toStdString());
        }
        root["+traits"] = array;
    }

    if (!patch->libraries.isEmpty())
    {
        nlohmann::json array;
        for (auto value: patch->libraries)
        {
            array.push_back(OneSixVersionFormat::libraryToJson(value.get()));
        }
        //root.insert("libraries", array);
        root["libraries"] = array;
    }
    if (!patch->mavenFiles.isEmpty())
    {
        nlohmann::json array;
        for (auto value: patch->mavenFiles)
        {
            array.push_back(OneSixVersionFormat::libraryToJson(value.get()));
        }
        //root.insert("mavenFiles", array);
        root["mavenFiles"] = array;
    }
    if (!patch->jarMods.isEmpty())
    {
        nlohmann::json array;
        for (auto value: patch->jarMods)
        {
            array.push_back(OneSixVersionFormat::jarModtoJson(value.get()));
        }
        root["jarMods"] = array;
    }
    if (!patch->mods.isEmpty())
    {
        nlohmann::json array;
        for (auto value: patch->jarMods)
        {
            array.push_back(OneSixVersionFormat::modtoJson(value.get()));
        }
        root["mods"] = array;
    }
    if(!patch->requires.empty())
    {
        Meta::serializeRequires(root, &patch->requires, "requires");
    }
    if(!patch->conflicts.empty())
    {
        Meta::serializeRequires(root, &patch->conflicts, "conflicts");
    }
    if(patch->m_volatile)
    {
        root["volatile"] = true;
    }
    // write the contents to a json document.
    {
        return root;
    }
}

LibraryPtr OneSixVersionFormat::plusJarModFromJson(
    ProblemContainer & problems,
    const nlohmann::json &libObj,
    const QString &filename,
    const QString &originalName
) {
    LibraryPtr out(new Library());
    if (!libObj.contains("name"))
    {
        throw Exception(filename + "contains a jarmod that doesn't have a 'name' field");
    }

    // just make up something unique on the spot for the library name.
    auto uuid = QUuid::createUuid();
    QString id = uuid.toString().remove('{').remove('}');
    out->setRawName(GradleSpecifier("org.multimc.jarmods:" + id + ":1"));

    // filename override is the old name
    //out->setFilename(libObj.value("name").toString());
    out->setFilename(QString::fromStdString(libObj["name"]));

    // it needs to be local, it is stored in the instance jarmods folder
    out->setHint("local");

    // read the original name if present - some versions did not set it
    // it is the original jar mod filename before it got renamed at the point of addition
    auto displayName = QString::fromStdString(libObj.get<std::string>());
    if(displayName.isEmpty())
    {
        auto fixed = originalName;
        fixed.remove(" (jar mod)");
        out->setDisplayName(fixed);
    }
    else
    {
        out->setDisplayName(displayName);
    }
    return out;
}

LibraryPtr OneSixVersionFormat::jarModFromJson(ProblemContainer & problems, const nlohmann::json& libObj, const QString& filename)
{
    return libraryFromJson(problems, libObj, filename);
}


nlohmann::json OneSixVersionFormat::jarModtoJson(Library *jarmod)
{
    return libraryToJson(jarmod);
}

LibraryPtr OneSixVersionFormat::modFromJson(ProblemContainer & problems, const nlohmann::json& libObj, const QString& filename)
{
    return  libraryFromJson(problems, libObj, filename);
}

nlohmann::json OneSixVersionFormat::modtoJson(Library *jarmod)
{
    return libraryToJson(jarmod);
}
