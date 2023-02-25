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

#include "MojangVersionFormat.h"
#include "OneSixVersionFormat.h"
#include "MojangDownloadInfo.h"
#include "Json.hpp"

#include "Json.h"
using namespace Json;
#include "ParseUtils.h"
#include <BuildConfig.h>

static const int CURRENT_MINIMUM_LAUNCHER_VERSION = 18;

static MojangAssetIndexInfo::Ptr assetIndexFromJson (const nlohmann::json &obj);
static MojangDownloadInfo::Ptr downloadInfoFromJson (const nlohmann::json &obj);
static MojangLibraryDownloadInfo::Ptr libDownloadInfoFromJson (const nlohmann::json &libObj);
static nlohmann::json assetIndexToJson (MojangAssetIndexInfo::Ptr assetidxinfo);
static nlohmann::json libDownloadInfoToJson (MojangLibraryDownloadInfo::Ptr libinfo);
static nlohmann::json downloadInfoToJson (MojangDownloadInfo::Ptr info);

namespace Bits
{
static void readString(const nlohmann::json &root, const QString &key, QString &variable)
{
    if (root.contains(key.toStdString()))
    {
        variable = QString::fromStdString(root[key.toStdString()].get<std::string>());
    }
}

static void readDownloadInfo(MojangDownloadInfo::Ptr out, const nlohmann::json &obj)
{
    // optional, not used
    readString(obj, "path", out->path);
    // required!
    out->sha1 = QString::fromStdString(obj["sha1"].get<std::string>());
    out->url = QString::fromStdString(obj["url"].get<std::string>());
    out->size = obj["size"].get<int>();
}

static void readAssetIndex(MojangAssetIndexInfo::Ptr out, const nlohmann::json &obj)
{
    out->totalSize = obj["totalSize"].get<int>();
    out->id = QString::fromStdString(obj["id"].get<std::string>());
    // out->known = true;
}
}

MojangDownloadInfo::Ptr downloadInfoFromJson(const nlohmann::json &obj)
{
    auto out = std::make_shared<MojangDownloadInfo>();
    Bits::readDownloadInfo(out, obj);
    return out;
}

MojangAssetIndexInfo::Ptr assetIndexFromJson(const nlohmann::json &obj)
{
    auto out = std::make_shared<MojangAssetIndexInfo>();
    Bits::readDownloadInfo(out, obj);
    Bits::readAssetIndex(out, obj);
    return out;
}

nlohmann::json downloadInfoToJson(MojangDownloadInfo::Ptr info)
{
    nlohmann::json out;
    if(!info->path.isNull())
    {
        //out.insert("path", info->path);
        out["path"] = info->path.toStdString();
    }
    //out.insert("sha1", info->sha1);
    //out.insert("size", info->size);
    //out.insert("url", info->url);
    out["sha1"] = info->sha1.toStdString();
    out["size"] = info->size;
    out["url"] = info->url.toStdString();

    return out;
}

MojangLibraryDownloadInfo::Ptr libDownloadInfoFromJson(const nlohmann::json &libObj)
{
    auto out = std::make_shared<MojangLibraryDownloadInfo>();
    auto dlObj = libObj["downloads"];
    if(dlObj.contains("artifact"))
    {
        out->artifact = downloadInfoFromJson(dlObj["artifact"]);
    }
    if(dlObj.contains("classifiers"))
    {
        auto classifiersObj = dlObj["classifiers"];
        for(auto iter = classifiersObj.begin(); iter != classifiersObj.end(); iter++)
        {
            /*
            auto classifier = iter.key();
            auto classifierObj = requireObject(iter.value());
            out->classifiers[classifier] = downloadInfoFromJson(classifierObj);
                */

            auto classifier = QString::fromStdString(iter.key());
            auto classifierObj = iter.value();
            out->classifiers[classifier] = downloadInfoFromJson(classifierObj);
        }
    }
    return out;
}

nlohmann::json libDownloadInfoToJson(MojangLibraryDownloadInfo::Ptr libinfo)
{
    nlohmann::json out;
    if(libinfo->artifact)
    {
        //out.insert("artifact", downloadInfoToJson(libinfo->artifact));
        out["artifact"] = downloadInfoToJson(libinfo->artifact);
    }
    if(libinfo->classifiers.size())
    {
        nlohmann::json classifiersOut;
        /*
        for(auto iter = libinfo->classifiers.begin(); iter != libinfo->classifiers.end(); iter++)
        {
            classifiersOut.insert(iter.key(), downloadInfoToJson(iter.value()));
        }
                */
        for (auto iter = libinfo->classifiers.begin(); iter != libinfo->classifiers.end(); iter++)
        {
            classifiersOut[iter.key().toStdString()] = downloadInfoToJson(iter.value());
        }
        //out.insert("classifiers", classifiersOut);
        out["classifiers"] = classifiersOut;
    }
    return out;
}

nlohmann::json assetIndexToJson(MojangAssetIndexInfo::Ptr info)
{
    nlohmann::json out;
    if(!info->path.isNull())
    {
        //out.insert("path", info->path
        out["path"] = info->path.toStdString();
    }
    /*
    out.insert("sha1", info->sha1);
    out.insert("size", info->size);
    out.insert("url", info->url);
    out.insert("totalSize", info->totalSize);
    out.insert("id", info->id);
        */

    out["sha1"] = info->sha1.toStdString();
    out["size"] = info->size;
    out["url"] = info->url.toStdString();
    out["totalSize"] = info->totalSize;
    out["id"] = info->id.toStdString();


    return out;
}

void MojangVersionFormat::readVersionProperties(const nlohmann::json &in, VersionFile *out)
{
    Bits::readString(in, "id", out->minecraftVersion);
    Bits::readString(in, "mainClass", out->mainClass);
    Bits::readString(in, "minecraftArguments", out->minecraftArguments);
    if(out->minecraftArguments.isEmpty())
    {
        QString processArguments;
        Bits::readString(in, "processArguments", processArguments);
        QString toCompare = processArguments.toLower();
        if (toCompare == "legacy")
        {
            out->minecraftArguments = " ${auth_player_name} ${auth_session}";
        }
        else if (toCompare == "username_session")
        {
            out->minecraftArguments = "--username ${auth_player_name} --session ${auth_session}";
        }
        else if (toCompare == "username_session_version")
        {
            out->minecraftArguments = "--username ${auth_player_name} --session ${auth_session} --version ${profile_name}";
        }
        else if (!toCompare.isEmpty())
        {
            out->addProblem(ProblemSeverity::Error, QObject::tr("processArguments is set to unknown value '%1'").arg(processArguments));
        }
    }
    Bits::readString(in, "type", out->type);

    Bits::readString(in, "assets", out->assets);
    if(in.contains("assetIndex"))
    {
        if (in["assetIndex"].is_object())
        {
            out->mojangAssetIndex = assetIndexFromJson(in["assetIndex"]);
        }
        else
        {
            throw std::runtime_error("assetIndex is not an object");
        }
    }
    else if (!out->assets.isNull())
    {
        out->mojangAssetIndex = std::make_shared<MojangAssetIndexInfo>(out->assets);
    }

    out->releaseTime = timeFromS3Time(QString::fromStdString(in.value("releaseTime", "")));
    out->updateTime = timeFromS3Time(QString::fromStdString(in.value("time", "")));

    if (in.contains("minimumLauncherVersion"))
    {
        out->minimumLauncherVersion = in.get<int>();
        if (out->minimumLauncherVersion > CURRENT_MINIMUM_LAUNCHER_VERSION)
        {
            out->addProblem(
                ProblemSeverity::Warning,
                QObject::tr("The 'minimumLauncherVersion' value of this version (%1) is higher than supported by %3 (%2). It might not work properly!")
                    .arg(out->minimumLauncherVersion)
                    .arg(CURRENT_MINIMUM_LAUNCHER_VERSION)
                    .arg(BuildConfig.LAUNCHER_NAME)
            );
        }
    }

    if (in.contains("compatibleJavaMajors"))
    {
        for (auto compatible : in["compatibleJavaMajors"])
        {
            out->compatibleJavaMajors.append(compatible.get<int>());
        }
    }

    if(in.contains("downloads"))
    {
        //auto downloadsObj = requireObject(in, "downloads");
        auto downloadsObj = in["downloads"];
        /*
        for(auto iter = downloadsObj.begin(); iter != downloadsObj.end(); iter++)
        {
            auto classifier = iter.key();
            auto classifierObj = requireObject(iter.value());
            out->mojangDownloads[classifier] = downloadInfoFromJson(classifierObj);
        }
        */

        for (auto iter = downloadsObj.begin(); iter != downloadsObj.end(); iter++)
        {
            QString classifier = QString::fromStdString(iter.key());
            auto classifierObj = iter.value();
            out->mojangDownloads[classifier] = downloadInfoFromJson(classifierObj);
        }
    }
}

VersionFilePtr MojangVersionFormat::versionFileFromJson(const nlohmann::json &doc, const QString &filename)
{
    VersionFilePtr out(new VersionFile());
    if (doc.empty() || doc.is_null())
    {
        throw std::runtime_error(filename.toStdString() + " is empty");
    }
    if (!doc.is_object())
    {
        throw std::runtime_error(filename.toStdString() + " is not an object");
    }

    //QJsonObject root = doc.object();

    readVersionProperties(doc, out.get());

    out->name = "Minecraft";
    out->uid = "net.minecraft";
    out->version = out->minecraftVersion;
    // out->filename = filename;


    if (doc.contains("libraries"))
    {
        for (auto libVal : doc["libraries"])
        {
            auto libObj = libVal;

            auto lib = MojangVersionFormat::libraryFromJson(*out, libObj, filename);
            out->libraries.append(lib);
        }
    }
    return out;
}

void MojangVersionFormat::writeVersionProperties(const VersionFile* in, nlohmann::json& out)
{
    /*
    writeString(out, "id", in->minecraftVersion);
    writeString(out, "mainClass", in->mainClass);
    writeString(out, "minecraftArguments", in->minecraftArguments);
    writeString(out, "type", in->type);
        */
    out["id"] = in->minecraftVersion.toStdString();
    out["mainClass"] = in->mainClass.toStdString();
    out["minecraftArguments"] = in->minecraftArguments.toStdString();
    out["type"] = in->type.toStdString();

    if(!in->releaseTime.isNull())
    {
        //writeString(out, "releaseTime", timeToS3Time(in->releaseTime));
        out["releaseTime"] = timeToS3Time(in->releaseTime).toStdString();
    }
    if(!in->updateTime.isNull())
    {
        //writeString(out, "time", timeToS3Time(in->updateTime));
        out["time"] = timeToS3Time(in->updateTime).toStdString();
    }
    if(in->minimumLauncherVersion != -1)
    {
        //out.insert("minimumLauncherVersion", in->minimumLauncherVersion);
        out["minimumLauncherVersion"] = in->minimumLauncherVersion;
    }
    //writeString(out, "assets", in->assets);
    out["assets"] = in->assets.toStdString();

    if(in->mojangAssetIndex && in->mojangAssetIndex->known)
    {
        //out.insert("assetIndex", assetIndexToJson(in->mojangAssetIndex));
        out["assetIndex"] = assetIndexToJson(in->mojangAssetIndex);
    }
    if(in->mojangDownloads.size())
    {
        //QJsonObject downloadsOut;
        nlohmann::json downloadsOut;
        /*
        for(auto iter = in->mojangDownloads.begin(); iter != in->mojangDownloads.end(); iter++)
        {
            downloadsOut.insert(iter.key(), downloadInfoToJson(iter.value()));
        }
        */
        for (auto iter = in->mojangDownloads.begin(); iter != in->mojangDownloads.end(); iter++)
        {
            downloadsOut[iter.key().toStdString()] = downloadInfoToJson(iter.value());
        }

        //out.insert("downloads", downloadsOut);
        out["downloads"] = downloadsOut;
    }
}

nlohmann::json MojangVersionFormat::versionFileToJson(const VersionFilePtr &patch)
{
    //QJsonObject root;
    nlohmann::json root;
    writeVersionProperties(patch.get(), root);
    if (!patch->libraries.isEmpty())
    {
        /*
        QJsonArray array;
        for (auto value: patch->libraries)
        {
            array.append(MojangVersionFormat::libraryToJson(value.get()));
        }
        root.insert("libraries", array);
        */
        nlohmann::json array;
        for (auto value: patch->libraries)
        {
            array.push_back(MojangVersionFormat::libraryToJson(value.get()));
        }

        root["libraries"] = array;
    }

    // write the contents to a json document.
    {

        //QJsonDocument out;
        //out.setObject(root);
        //return out;
        return root;
    }
}

LibraryPtr MojangVersionFormat::libraryFromJson(ProblemContainer & problems, const nlohmann::json &libObj, const QString &filename)
{
    LibraryPtr out(new Library());
    if (!libObj.contains("name"))
    {
        throw std::runtime_error((filename + "contains a library that doesn't have a 'name' field").toStdString());
    }
    auto rawName = QString::fromStdString(libObj["name"].get<std::string>());
    out->m_name = rawName;
    if(!out->m_name.valid()) {
        problems.addProblem(ProblemSeverity::Error, QObject::tr("Library %1 name is broken and cannot be processed.").arg(rawName));
    }

    Bits::readString(libObj, "url", out->m_repositoryURL);
    if (libObj.contains("extract"))
    {
        out->m_hasExcludes = true;
        auto extractObj = libObj["extract"];
        for (auto excludeVal : extractObj["exclude"])
        {
            out->m_extractExcludes.append(QString::fromStdString(excludeVal.get<std::string>()));
        }
    }
    if (libObj.contains("natives"))
    {
        //QJsonObject nativesObj = requireObject(libObj.value("natives"));
        auto nativesObj = libObj["natives"];
        for (auto it = nativesObj.begin(); it != nativesObj.end(); ++it)
        {
            if (!it.value().is_string())
            {
                qWarning() << filename << "contains an invalid native (skipping)";
            }
            // FIXME: Skip unknown platforms
            //out->m_nativeClassifiers[it.key()] = it.value().toString();
            out->m_nativeClassifiers[QString::fromStdString(it.key())] = QString::fromStdString(it.value().get<std::string>());
        }
    }
    if (libObj.contains("rules"))
    {
        out->applyRules = true;
        out->m_rules = rulesFromJsonV4(libObj);
    }
    if (libObj.contains("downloads"))
    {
        out->m_mojangDownloads = libDownloadInfoFromJson(libObj);
    }
    return out;
}

nlohmann::json MojangVersionFormat::libraryToJson(Library *library)
{
    nlohmann::json libRoot;
    //libRoot.insert("name", library->m_name.serialize());
    libRoot["name"] = library->m_name.serialize().toStdString();
    if (!library->m_repositoryURL.isEmpty())
    {
        //libRoot.insert("url", library->m_repositoryURL);
        libRoot["url"] = library->m_repositoryURL.toStdString();
    }
    if (library->isNative())
    {
        nlohmann::json nativeList;
        auto iter = library->m_nativeClassifiers.begin();
        while (iter != library->m_nativeClassifiers.end())
        {
            //nativeList.insert(iter.key(), iter.value());
            //iter++;
            nativeList[iter.key().toStdString()] = iter.value().toStdString();
            iter++;
        }
        //libRoot.insert("natives", nativeList);
        if (library->m_extractExcludes.size())
        {
            /*
            QJsonArray excludes;
            QJsonObject extract;
            for (auto exclude : library->m_extractExcludes)
            {
                excludes.append(exclude);
            }
            extract.insert("exclude", excludes);
            libRoot.insert("extract", extract);
            */
            nlohmann::json excludes;
            nlohmann::json extract;
            for (auto exclude : library->m_extractExcludes)
            {
                excludes.push_back(exclude.toStdString());
            }
            extract["exclude"] = excludes;
            libRoot["extract"] = extract;

        }
    }
    if (library->m_rules.size())
    {
        /*
        QJsonArray allRules;
        for (auto &rule : library->m_rules)
        {
            QJsonObject ruleObj = rule->toJson();
            allRules.append(ruleObj);
        }
        libRoot.insert("rules", allRules);
        */
        nlohmann::json allRules;
        for (auto &rule : library->m_rules)
        {
            nlohmann::json ruleObj = rule->toJson();
            allRules.push_back(ruleObj);
        }
        libRoot["rules"] = allRules;
    }
    if(library->m_mojangDownloads)
    {
        /*
        auto downloadsObj = libDownloadInfoToJson(library->m_mojangDownloads);
        libRoot.insert("downloads", downloadsObj);
        */
        auto downloadsObj = libDownloadInfoToJson(library->m_mojangDownloads);
        libRoot["downloads"] = downloadsObj;
    }
    return libRoot;
}
