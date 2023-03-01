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

#include "AccountData.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QRegularExpression>

namespace {

nlohmann::json convertQMapToJsonObject(const QMap<QString, QVariant>& map, const nlohmann::json& in) {
    nlohmann::json out;

    // better way to do this?
    for (auto it = map.begin(); it != map.end(); ++it) {
        switch (it.value().type()) {
            case QVariant::Type::Map:
                out[it.key().toStdString()] = convertQMapToJsonObject(it.value().toMap(), in);
                break;
            case QVariant::Type::String:
                out[it.key().toStdString()] = it.value().toString().toStdString();
                break;
            case QVariant::Type::Bool:
                out[it.key().toStdString()] = it.value().toBool();
                break;
            case QVariant::Type::Int:
                out[it.key().toStdString()] = it.value().toInt();
                break;
            case QVariant::Type::Double:
                out[it.key().toStdString()] = it.value().toDouble();
                break;
            default:
                out[it.key().toStdString()] = it.value().toString().toStdString();
                break;
        }
    }

    return out;
}

void tokenToJSONV3(nlohmann::json& parent, const Katabasis::Token& t, const char* tokenName) {
    if(!t.persistent) return;

    nlohmann::json out;
    if(t.issueInstant.isValid()) {
        out["iat"] = t.issueInstant.toMSecsSinceEpoch() / 1000;
    }

    if(t.notAfter.isValid()) {
        out["exp"] = t.notAfter.toMSecsSinceEpoch() / 1000;
    }

    bool save = false;
    if(!t.token.isEmpty()) {
        out["token"] = t.token.toStdString();
        save = true;
    }
    if(!t.refresh_token.isEmpty()) {
        out["refresh_token"] = t.refresh_token.toStdString();
        save = true;
    }
    if(!t.extra.empty()) {
        //out["extra"] = t.extra.toStdMap();
        out["extra"] = convertQMapToJsonObject(t.extra, out);
        save = true;
    }
    if(save) {
        parent[tokenName] = out;
    }
}

Katabasis::Token tokenFromJSONV3(const QJsonObject &parent, const char * tokenName) {
    Katabasis::Token out;
    auto tokenObject = parent.value(tokenName).toObject();
    if(tokenObject.isEmpty()) {
        return out;
    }
    auto issueInstant = tokenObject.value("iat");
    if(issueInstant.isDouble()) {
        out.issueInstant = QDateTime::fromMSecsSinceEpoch(((int64_t) issueInstant.toDouble()) * 1000);
    }

    auto notAfter = tokenObject.value("exp");
    if(notAfter.isDouble()) {
        out.notAfter = QDateTime::fromMSecsSinceEpoch(((int64_t) notAfter.toDouble()) * 1000);
    }

    auto token = tokenObject.value("token");
    if(token.isString()) {
        out.token = token.toString();
        out.validity = Katabasis::Validity::Assumed;
    }

    auto refresh_token = tokenObject.value("refresh_token");
    if(refresh_token.isString()) {
        out.refresh_token = refresh_token.toString();
    }

    auto extra = tokenObject.value("extra");
    if(extra.isObject()) {
        out.extra = extra.toObject().toVariantMap();
    }
    return out;
}

void profileToJSONV3(nlohmann::json &parent, MinecraftProfile p, const char* tokenName) {
    if(p.id.isEmpty()) return;

    nlohmann::json out;
    out["id"] = p.id.toStdString();
    out["name"] = p.name.toStdString();
    if(!p.currentCape.isEmpty()) {
        out["cape"] = p.currentCape.toStdString();
    }

    {
        nlohmann::json skinObj;
        skinObj["id"] = p.skin.id.toStdString();
        skinObj["url"] =  p.skin.url.toStdString();
        skinObj["variant"] = p.skin.variant.toStdString();
        if(p.skin.data.size()) {
            skinObj["data"] = QString::fromLatin1(p.skin.data.toBase64()).toStdString();
        }
        out["skin"] = skinObj;
    }

    nlohmann::json capesArray;
    for(auto & cape: p.capes) {
        nlohmann::json capeObj;
        capeObj["id"] = cape.id.toStdString();
        capeObj["url"] = cape.url.toStdString();
        capeObj["alias"] = cape.alias.toStdString();
        if(cape.data.size()) {
            capeObj["data"] = QString::fromLatin1(cape.data.toBase64()).toStdString();
        }
        capesArray.push_back(capeObj);
    }
    out["capes"] = capesArray;
    parent[tokenName] = out;
}

MinecraftProfile profileFromJSONV3(const nlohmann::json& parent, const char* tokenName) {
    MinecraftProfile out;
    auto tokenObject = parent[tokenName];

    const QRegularExpression base64Regex("^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=|[A-Za-z0-9+/]{4})$");

    if(tokenObject.empty())
        return out;

    //the .contains checks are probably redundant, .value() will probably suffice
    {
        if (!tokenObject.contains("id") || !tokenObject.contains("name")) {
            qWarning() << "mandatory profile attributes are missing or of unexpected type";
            return MinecraftProfile();
        }

        auto idV = tokenObject["id"];
        auto nameV = tokenObject["name"];

        if (idV.is_string() || nameV.is_string()) {
            qWarning() << "mandatory profile attributes are missing or of unexpected type";
            return MinecraftProfile();
        }

        out.name = QString::fromStdString(nameV);
        out.id = QString::fromStdString(idV);
    }

    {
        if (!tokenObject.contains("skin") || !tokenObject["skin"].is_object()) {
            qWarning() << "skin is missing";
            return MinecraftProfile();
        }

        auto skinObj = tokenObject["skin"];

        if (!skinObj.contains("id") || !skinObj.contains("url") || !skinObj.contains("variant")) {
            qWarning() << "mandatory skin attributes are missing or of unexpected type";
            return MinecraftProfile();
        }

        out.skin.id = QString::fromStdString(skinObj["id"].get<std::string>());
        out.skin.url = QString::fromStdString(skinObj["url"].get<std::string>());
        out.skin.variant = QString::fromStdString(skinObj["variant"].get<std::string>());

        // data for skin is optional
        if (skinObj.contains("data")) {
            auto dataV = skinObj["data"];

            if (dataV.is_string()) {
                QString data = QString::fromStdString(dataV.get<std::string>());
                if (base64Regex.match(data).hasMatch()) {
                    out.skin.data = QByteArray::fromBase64(data.toLatin1());
                } else {
                    qWarning() << "skin data is not base64";
                    return MinecraftProfile();
                }
            } else {
                qWarning() << "skin data is something unexpected";
                return MinecraftProfile();
            }
        }
    }

    {
        if (!tokenObject.contains("capes") || !tokenObject["capes"].is_array()) {
            qWarning() << "capes is missing";
            return MinecraftProfile();
        }

        auto capesArray = tokenObject["capes"];

        for(const auto& capeV: capesArray) {
            if(!capeV.is_object()) {
                qWarning() << "cape is not an object!";
                return MinecraftProfile();
            }

            if (!capeV.contains("id") || !capeV.contains("url") || !capeV.contains("alias")) {
                qWarning() << "mandatory cape attributes are missing or of unexpected type";
                return MinecraftProfile();
            }

            Cape cape;
            cape.id = QString::fromStdString(capeV["id"].get<std::string>());
            cape.url = QString::fromStdString(capeV["url"].get<std::string>());
            cape.alias = QString::fromStdString(capeV["alias"].get<std::string>());

            // data for cape is optional.
            if (capeV.contains("data")) {
                auto dataV = capeV["data"];

                if (dataV.is_string()) {
                    QString data = QString::fromStdString(dataV.get<std::string>());
                    if (base64Regex.match(data).hasMatch()) {
                        cape.data = QByteArray::fromBase64(data.toLatin1());
                    } else {
                        qWarning() << "cape data is not base64";
                        return MinecraftProfile();
                    }
                } else {
                    qWarning() << "cape data is something unexpected";
                    return MinecraftProfile();
                }
            }
            out.capes[cape.id] = cape;
        }
    }
    // current cape
    {
        if (tokenObject.contains("cape")) {
            auto capeV = tokenObject["cape"];
            if (capeV.is_string()) {
                auto currentCape = QString::fromStdString(capeV.get<std::string>());
                if (out.capes.contains(currentCape)) {
                    out.currentCape = currentCape;
                }
            }
        }
    }
    out.validity = Katabasis::Validity::Assumed;
    return out;
}

void entitlementToJSONV3(QJsonObject &parent, MinecraftEntitlement p) {
    if(p.validity == Katabasis::Validity::None) {
        return;
    }
    QJsonObject out;
    out["ownsMinecraft"] = QJsonValue(p.ownsMinecraft);
    out["canPlayMinecraft"] = QJsonValue(p.canPlayMinecraft);
    parent["entitlement"] = out;
}

bool entitlementFromJSONV3(const QJsonObject &parent, MinecraftEntitlement & out) {
    auto entitlementObject = parent.value("entitlement").toObject();
    if(entitlementObject.isEmpty()) {
        return false;
    }
    {
        auto ownsMinecraftV = entitlementObject.value("ownsMinecraft");
        auto canPlayMinecraftV = entitlementObject.value("canPlayMinecraft");
        if(!ownsMinecraftV.isBool() || !canPlayMinecraftV.isBool()) {
            qWarning() << "mandatory attributes are missing or of unexpected type";
            return false;
        }
        out.canPlayMinecraft = canPlayMinecraftV.toBool(false);
        out.ownsMinecraft = ownsMinecraftV.toBool(false);
        out.validity = Katabasis::Validity::Assumed;
    }
    return true;
}

}

bool AccountData::resumeStateFromV2(const nlohmann::json& data) {
    // The JSON object must at least have a username for it to be valid.
    if (!data.contains("username") || !data["username"].is_string())
    {
        qCritical() << "Can't load Mojang account info from JSON object. Username field is missing or of the wrong type.";
        return false;
    }

    QString userName = data.value("username").toString("");
    QString clientToken = data.value("clientToken").toString("");
    QString accessToken = data.value("accessToken").toString("");

    QJsonArray profileArray = data.value("profiles").toArray();
    if (profileArray.size() < 1)
    {
        qCritical() << "Can't load Mojang account with username \"" << userName << "\". No profiles found.";
        return false;
    }

    struct AccountProfile
    {
        QString id;
        QString name;
        bool legacy;
    };

    QList<AccountProfile> profiles;
    int currentProfileIndex = 0;
    int index = -1;
    QString currentProfile = data.value("activeProfile").toString("");
    for (QJsonValue profileVal : profileArray)
    {
        index++;
        QJsonObject profileObject = profileVal.toObject();
        QString id = profileObject.value("id").toString("");
        QString name = profileObject.value("name").toString("");
        bool legacy = profileObject.value("legacy").toBool(false);
        if (id.isEmpty() || name.isEmpty())
        {
            qWarning() << "Unable to load a profile" << name << "because it was missing an ID or a name.";
            continue;
        }
        if(id == currentProfile) {
            currentProfileIndex = index;
        }
        profiles.append({id, name, legacy});
    }
    auto & profile = profiles[currentProfileIndex];

    type = AccountType::Mojang;
    legacy = profile.legacy;

    minecraftProfile.id = profile.id;
    minecraftProfile.name = profile.name;
    minecraftProfile.validity = Katabasis::Validity::Assumed;

    yggdrasilToken.token = accessToken;
    yggdrasilToken.extra["clientToken"] = clientToken;
    yggdrasilToken.extra["userName"] = userName;
    yggdrasilToken.validity = Katabasis::Validity::Assumed;

    validity_ = minecraftProfile.validity;
    return true;
}

bool AccountData::resumeStateFromV3(const nlohmann::json& data) {
    auto typeV = data.value("type");
    if(!typeV.isString()) {
        qWarning() << "Failed to parse account data: type is missing.";
        return false;
    }
    auto typeS = typeV.toString();
    if(typeS == "MSA") {
        type = AccountType::MSA;
    } else if (typeS == "Mojang") {
        type = AccountType::Mojang;
    } else if (typeS == "Offline") {
        type = AccountType::Offline;
    } else {
        qWarning() << "Failed to parse account data: type is not recognized.";
        return false;
    }

    if(type == AccountType::Mojang) {
        legacy = data.value("legacy").toBool(false);
        canMigrateToMSA = data.value("canMigrateToMSA").toBool(false);
    }

    if(type == AccountType::MSA) {
        auto clientIDV = data.value("msa-client-id");
        if (clientIDV.isString()) {
            msaClientID = clientIDV.toString();
        } // leave msaClientID empty if it doesn't exist or isn't a string
        msaToken = tokenFromJSONV3(data, "msa");
        userToken = tokenFromJSONV3(data, "utoken");
        xboxApiToken = tokenFromJSONV3(data, "xrp-main");
        mojangservicesToken = tokenFromJSONV3(data, "xrp-mc");
    }

    yggdrasilToken = tokenFromJSONV3(data, "ygg");
    minecraftProfile = profileFromJSONV3(data, "profile");
    if(!entitlementFromJSONV3(data, minecraftEntitlement)) {
        if(minecraftProfile.validity != Katabasis::Validity::None) {
            minecraftEntitlement.canPlayMinecraft = true;
            minecraftEntitlement.ownsMinecraft = true;
            minecraftEntitlement.validity = Katabasis::Validity::Assumed;
        }
    }

    validity_ = minecraftProfile.validity;
    return true;
}

nlohmann::json AccountData::saveState() const {
    nlohmann::json output;
    if(type == AccountType::Mojang) {
        output["type"] = "Mojang";
        if(legacy) {
            output["legacy"] = true;
        }
        if(canMigrateToMSA) {
            output["canMigrateToMSA"] = true;
        }
    }
    else if (type == AccountType::MSA) {
        output["type"] = "MSA";
        output["msa-client-id"] = msaClientID.toStdString();
        tokenToJSONV3(output, msaToken, "msa");
        tokenToJSONV3(output, userToken, "utoken");
        tokenToJSONV3(output, xboxApiToken, "xrp-main");
        tokenToJSONV3(output, mojangservicesToken, "xrp-mc");
    }
    else if (type == AccountType::Offline) {
        output["type"] = "Offline";
    }

    tokenToJSONV3(output, yggdrasilToken, "ygg");
    profileToJSONV3(output, minecraftProfile, "profile");
    entitlementToJSONV3(output, minecraftEntitlement);
    return output;
}

QString AccountData::userName() const {
    if(type == AccountType::MSA) {
        return QString();
    }
    return yggdrasilToken.extra["userName"].toString();
}

QString AccountData::accessToken() const {
    return yggdrasilToken.token;
}

QString AccountData::clientToken() const {
    if(type != AccountType::Mojang) {
        return QString();
    }
    return yggdrasilToken.extra["clientToken"].toString();
}

void AccountData::setClientToken(QString clientToken) {
    if(type != AccountType::Mojang) {
        return;
    }
    yggdrasilToken.extra["clientToken"] = clientToken;
}

void AccountData::generateClientTokenIfMissing() {
    if(yggdrasilToken.extra.contains("clientToken")) {
        return;
    }
    invalidateClientToken();
}

void AccountData::invalidateClientToken() {
    if(type != AccountType::Mojang) {
        return;
    }
    yggdrasilToken.extra["clientToken"] = QUuid::createUuid().toString().remove(QRegularExpression("[{-}]"));
}

QString AccountData::profileId() const {
    return minecraftProfile.id;
}

QString AccountData::profileName() const {
    if(minecraftProfile.name.size() == 0) {
        return QObject::tr("No profile (%1)").arg(accountDisplayString());
    }
    else {
        return minecraftProfile.name;
    }
}

QString AccountData::accountDisplayString() const {
    switch(type) {
        case AccountType::Mojang: {
            return userName();
        }
        case AccountType::Offline: {
            return QObject::tr("<Offline>");
        }
        case AccountType::MSA: {
            if(xboxApiToken.extra.contains("gtg")) {
                return xboxApiToken.extra["gtg"].toString();
            }
            return "Xbox profile missing";
        }
        default: {
            return "Invalid Account";
        }
    }
}

QString AccountData::lastError() const {
    return errorString;
}
