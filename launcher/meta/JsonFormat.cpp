/* Copyright 2015-2021 MultiMC Contributors
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

#include "JsonFormat.h"

// FIXME: remove this from here... somehow
#include "minecraft/OneSixVersionFormat.h"
#include "Json.h"
#include "Json.hpp"

#include "Index.h"
#include "Version.h"
#include "VersionList.h"

using namespace Json;

namespace Meta
{

MetadataVersion currentFormatVersion()
{
    return MetadataVersion::InitialRelease;
}

// Index
static std::shared_ptr<Index> parseIndexInternal(const nlohmann::json &obj)
{
    /*
    const QVector<QJsonObject> objects = requireIsArrayOf<QJsonObject>(obj, "packages");
    QVector<VersionListPtr> lists;
    lists.reserve(objects.size());
    std::transform(objects.begin(), objects.end(), std::back_inserter(lists), [](const QJsonObject &obj)
    {
        VersionListPtr list = std::make_shared<VersionList>(requireString(obj, "uid"));
        list->setName(ensureString(obj, "name", QString()));
        return list;
    });
    return std::make_shared<Index>(lists);
        */

    QVector<nlohmann::json> objects;

    if (obj.contains("packages"))
    {
        //ensure that the packages are an array
        if (!obj["packages"].is_array())
        {
            throw ParseException("packages is not an array");
        }
        else
        {
            //iterate over the array and add each object to the vector
            for (auto &package : obj["packages"])
            {
                    objects.push_back(package);
            }
        }
    }

    QVector<VersionListPtr> lists;
    lists.reserve(objects.size());

    std::transform(objects.begin(), objects.end(), std::back_inserter(lists), [](const nlohmann::json &object)
    {
        VersionListPtr list = std::make_shared<VersionList>(QString::fromStdString(object["uid"]));
        list->setName(QString::fromStdString(object.value("name", "")));
        return list;
    });

    return std::make_shared<Index>(lists);
}

// Version
static VersionPtr parseCommonVersion(const QString &uid, const nlohmann::json &obj)
{
    /*
    VersionPtr version = std::make_shared<Version>(uid, requireString(obj, "version"));
    version->setTime(QDateTime::fromString(requireString(obj, "releaseTime"), Qt::ISODate).toMSecsSinceEpoch() / 1000);
    version->setType(ensureString(obj, "type", QString()));
    version->setRecommended(ensureBoolean(obj, QString("recommended"), false));
    version->setVolatile(ensureBoolean(obj, QString("volatile"), false));
    RequireSet requires, conflicts;
    parseRequires(obj, &requires, "requires");
    parseRequires(obj, &conflicts, "conflicts");
    version->setRequires(requires, conflicts);
    return version;
        */

    VersionPtr version = std::make_shared<Version>(uid, QString::fromStdString(obj["version"]));
    version->setTime(QDateTime::fromString(QString::fromStdString(obj["releaseTime"]), Qt::ISODate).toMSecsSinceEpoch() / 1000);
    version->setType(QString::fromStdString(obj.value("type", "")));
    version->setRecommended(obj.value("recommended", false));
    version->setVolatile(obj.value("volatile", false));
    RequireSet requires, conflicts;
    parseRequires(obj, &requires, "requires");
    parseRequires(obj, &conflicts, "conflicts");
    version->setRequires(requires, conflicts);
    return version;
}

static std::shared_ptr<Version> parseVersionInternal(const nlohmann::json &obj)
{
    //VersionPtr version = parseCommonVersion(requireString(obj, "uid"), obj);
    VersionPtr version = parseCommonVersion(QString::fromStdString(obj["uid"]), obj);

    version->setData(OneSixVersionFormat::versionFileFromJson(obj,
                                           QString("%1/%2.json").arg(version->uid(), version->version()),
                                           obj.contains("order")));
    return version;
}

// Version list / package
static std::shared_ptr<VersionList> parseVersionListInternal(const nlohmann::json &obj)
{
    const QString uid = QString::fromStdString(obj["uid"]);

    /*
    const QVector<QJsonObject> versionsRaw = requireIsArrayOf<QJsonObject>(obj, "versions");
    QVector<VersionPtr> versions;
    versions.reserve(versionsRaw.size());
    std::transform(versionsRaw.begin(), versionsRaw.end(), std::back_inserter(versions), [uid](const QJsonObject &vObj)
    {
        auto version = parseCommonVersion(uid, vObj);
        version->setProvidesRecommendations();
        return version;
    });

    VersionListPtr list = std::make_shared<VersionList>(uid);
    list->setName(ensureString(obj, "name", QString()));
    list->setVersions(versions);
    return list;
        */

    QVector<nlohmann::json> versionsRaw;

    if (obj.contains("versions"))
    {
        //ensure that the versions are an array
        if (!obj["versions"].is_array())
        {
            throw ParseException("versions is not an array");
        }
        else
        {
            //iterate over the array and add each object to the vector
            for (auto &version : obj["versions"])
            {
                versionsRaw.push_back(version);
            }
        }
    }

    QVector<VersionPtr> versions;
    versions.reserve(versionsRaw.size());

    std::transform(versionsRaw.begin(), versionsRaw.end(), std::back_inserter(versions), [uid](const nlohmann::json &vObj)
    {
        auto version = parseCommonVersion(uid, vObj);
        version->setProvidesRecommendations();
        return version;
    });

    VersionListPtr list = std::make_shared<VersionList>(uid);
    list->setName(QString::fromStdString(obj.value("name", "")));
    list->setVersions(versions);

    return list;
}


MetadataVersion parseFormatVersion(const nlohmann::json &obj, bool required)
{
    if (!obj.contains("formatVersion"))
    {
        if(required)
        {
            return MetadataVersion::Invalid;
        }
        return MetadataVersion::InitialRelease;
    }

    if (!obj["formatVersion"].is_number_integer())
    {
        return MetadataVersion::Invalid;
    }

    switch(obj["formatVersion"].get<int>())
    {
        case 0:
        case 1:
            return MetadataVersion::InitialRelease;
        default:
            return MetadataVersion::Invalid;
    }
}

void serializeFormatVersion(QJsonObject& obj, Meta::MetadataVersion version)
{
    if(version == MetadataVersion::Invalid)
    {
        return;
    }
    obj.insert("formatVersion", int(version));
}

void parseIndex(const nlohmann::json &obj, Index *ptr)
{
    const MetadataVersion version = parseFormatVersion(obj);
    switch (version)
    {
    case MetadataVersion::InitialRelease:
        ptr->merge(parseIndexInternal(obj));
        break;
    case MetadataVersion::Invalid:
        throw ParseException(QObject::tr("Unknown format version!"));
    }
}

void parseVersionList(const nlohmann::json &obj, VersionList *ptr)
{
    const MetadataVersion version = parseFormatVersion(obj);
    switch (version)
    {
    case MetadataVersion::InitialRelease:
        ptr->merge(parseVersionListInternal(obj));
        break;
    case MetadataVersion::Invalid:
        throw ParseException(QObject::tr("Unknown format version!"));
    }
}

void parseVersion(const nlohmann::json &obj, Version *ptr)
{
    const MetadataVersion version = parseFormatVersion(obj);
    switch (version)
    {
    case MetadataVersion::InitialRelease:
        ptr->merge(parseVersionInternal(obj));
        break;
    case MetadataVersion::Invalid:
        throw ParseException(QObject::tr("Unknown format version!"));
    }
}

/*
[
{"uid":"foo", "equals":"version"}
]
*/
void parseRequires(const nlohmann::json& obj, RequireSet* ptr, const char * keyName)
{
    if(obj.contains(keyName))
    {

       QSet<QString> requires;
       auto reqArray = obj[keyName];
        for (auto &reqObject : reqArray)
        {
            auto uid = QString::fromStdString(reqObject["uid"]);
            auto equals = QString::fromStdString(reqObject.value("equals", ""));
            auto suggests = QString::fromStdString(reqObject.value("suggests", ""));
            ptr->insert({uid, equals, suggests});
        }
    }
}
void serializeRequires(nlohmann::json& obj, RequireSet* ptr, const char * keyName)
{
    if(!ptr || ptr->empty())
    {
        return;
    }
    nlohmann::json arrOut;
    for(auto &iter: *ptr)
    {
        /*
        QJsonObject reqOut;
        reqOut.insert("uid", iter.uid);
        if(!iter.equalsVersion.isEmpty())
        {
            reqOut.insert("equals", iter.equalsVersion);
        }
        if(!iter.suggests.isEmpty())
        {
            reqOut.insert("suggests", iter.suggests);
        }
        arrOut.append(reqOut);
        */
        nlohmann::json reqOut;
        reqOut["uid"] = iter.uid.toStdString();
        if(!iter.equalsVersion.isEmpty())
        {
            reqOut["equals"] = iter.equalsVersion.toStdString();
        }
        if(!iter.suggests.isEmpty())
        {
            reqOut["suggests"] = iter.suggests.toStdString();
        }
        arrOut.push_back(reqOut);
    }
    obj[keyName] = arrOut;
}

}

