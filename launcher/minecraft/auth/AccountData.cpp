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

nlohmann::json convertQMapToJsonObject(const QMap<QString, QVariant>& map) {
    QByteArray cborArray = QCborValue::fromVariant(map).toCbor();
    return nlohmann::json::from_cbor(cborArray);
}

QMap<QString, QVariant> convertJsonObjectToQMap(const nlohmann::json& json) {
    std::vector<std::uint8_t> cbor = nlohmann::json::to_cbor(json);

    QCborValue cborValue = QCborValue::fromCbor(cbor.data(), cbor.size());

    return cborValue.toVariant().toMap();
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
        out["extra"] = convertQMapToJsonObject(t.extra);
        save = true;
    }
    if(save) {
        parent[tokenName] = out;
    }
}

Katabasis::Token tokenFromJSONV3(const nlohmann::json& parent, const char* tokenName) {
    Katabasis::Token out;
    if(!parent.contains(tokenName))
        return out;

    const auto& tokenObject = parent[tokenName];

    auto issueInstant = tokenObject.value("iat", nlohmann::json{});
    if(issueInstant.is_number_float()) {
        out.issueInstant = QDateTime::fromMSecsSinceEpoch(((int64_t) issueInstant.get<float>()) * 1000);
    }

    auto notAfter = tokenObject.value("exp", nlohmann::json{});
    if(notAfter.is_number_float()) {
        out.notAfter = QDateTime::fromMSecsSinceEpoch(((int64_t) notAfter.get<float>()) * 1000);
    }

    auto token = tokenObject.value("token", nlohmann::json{});
    if(token.is_string()) {
        out.token = token.get<std::string>().c_str();
        out.validity = Katabasis::Validity::Assumed;
    }

    auto refresh_token = tokenObject.value("refresh_token", nlohmann::json{});
    if(refresh_token.is_string()) {
        out.refresh_token = refresh_token.get<std::string>().c_str();
    }

    auto extra = tokenObject.value("extra", nlohmann::json{});
    if(extra.is_object()) {
        out.extra = convertJsonObjectToQMap(extra);
    }

    return out;
}

void profileToJSONV3(nlohmann::json &parent, const MinecraftProfile& p, const char* tokenName) {
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

    nlohmann::json capesArray = nlohmann::json::array();
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
    const auto tokenObject = parent.value(tokenName, nlohmann::json{});
    const QRegularExpression base64Regex("^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=|[A-Za-z0-9+/]{4})$");

    if(tokenObject.empty())
        return out;

    {
        auto idV = tokenObject.value("id", nlohmann::json{});
        auto nameV = tokenObject.value("name", nlohmann::json{});
        if (!idV.is_string() || !nameV.is_string()) {
            qWarning() << "mandatory profile attributes are missing or of unexpected type";
            return MinecraftProfile();
        }

        out.name = nameV.get<std::string>().c_str();
        out.id = idV.get<std::string>().c_str();
    }

    {
        auto skinObj = tokenObject.value("skin", nlohmann::json{});
        if(skinObj.empty())
        {
            qWarning() << "skin is missing";
            return MinecraftProfile();
        }

        auto idV = skinObj.value("id", nlohmann::json{});
        auto urlV = skinObj.value("url", nlohmann::json{});
        auto variantV = skinObj.value("variant", nlohmann::json{});
        if (!idV.is_string() || !urlV.is_string() || !variantV.is_string()) {
            qWarning() << "mandatory skin attributes are missing or of unexpected type";
            return MinecraftProfile();
        }

        out.skin.id = skinObj["id"].get<std::string>().c_str();
        out.skin.url = skinObj["url"].get<std::string>().c_str();
        out.skin.variant = skinObj["variant"].get<std::string>().c_str();
        if (skinObj.contains("data")) {
            auto dataV = skinObj["data"];

            if (dataV.is_string()) {
                QString data = dataV.get<std::string>().c_str();
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
        auto capesArray = tokenObject.value("capes", nlohmann::json{});
        if(!capesArray.is_array() || capesArray.is_null())
        {
            qWarning() << "capes is missing";
            return MinecraftProfile();
        }

        for(const auto& capeV: capesArray) {
            if(!capeV.is_object()) {
                qWarning() << "cape is not an object!";
                return MinecraftProfile();
            }

            auto idV = capeV.value("id", nlohmann::json{});
            auto urlV = capeV.value("url", nlohmann::json{});
            auto aliasV = capeV.value("alias", nlohmann::json{});
            if (!idV.is_string() || !urlV.is_string() || !aliasV.is_string()) {
                qWarning() << "mandatory cape attributes are missing or of unexpected type";
                return MinecraftProfile();
            }
            Cape cape;
            cape.id = idV.get<std::string>().c_str();
            cape.url = urlV.get<std::string>().c_str();
            cape.alias = aliasV.get<std::string>().c_str();

            // data for cape is optional.
            auto dataV = capeV.value("data", nlohmann::json{});
            if(!dataV.empty())
            {
                if (dataV.is_string()) {
                    QString data = dataV.get<std::string>().c_str();
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
        auto capeV = tokenObject.value("cape", nlohmann::json{});
        if(!capeV.empty())
        {
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

void entitlementToJSONV3(nlohmann::json& parent, MinecraftEntitlement p) {
    if(p.validity == Katabasis::Validity::None) {
        return;
    }
    nlohmann::json out;

    out["ownsMinecraft"] = p.ownsMinecraft;
    out["canPlayMinecraft"] = p.canPlayMinecraft;
    parent["entitlement"] = out;
}

bool entitlementFromJSONV3(const nlohmann::json& parent, MinecraftEntitlement& out) {
    auto entitlementObject = parent.value("entitlement", nlohmann::json{});
    if(entitlementObject.empty())
        return false;

    {
        auto ownsMinecraftV = entitlementObject.value("ownsMinecraft", nlohmann::json{});
        auto canPlayMinecraftV = entitlementObject.value("canPlayMinecraft", nlohmann::json{});
        if(!ownsMinecraftV.is_boolean() || !canPlayMinecraftV.is_boolean()) {
            qWarning() << "mandatory attributes are missing or of unexpected type";
            return false;
        }
        out.canPlayMinecraft = canPlayMinecraftV.get<bool>();
        out.ownsMinecraft = ownsMinecraftV.get<bool>();
        out.validity = Katabasis::Validity::Assumed;
    }
    return true;
}

bool AccountData::resumeStateFromV2(const nlohmann::json& data) {
    // The JSON object must at least have a username for it to be valid.
    if (!data.contains("username") || !data["username"].is_string())
    {
        qCritical() << "Can't load Mojang account info from JSON object. Username field is missing or of the wrong type.";
        return false;
    }

    QString userName = data.value("username", "").c_str();
    QString clientToken = data.value("clientToken", "").c_str();
    QString accessToken = data.value("accessToken", "").c_str();

    nlohmann::json profileArray = data.value("profiles", nlohmann::json::array());
    if (profileArray.empty())
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
    QString currentProfile = data.value("activeProfile", "").c_str();
    for (const nlohmann::json& profileVal : profileArray)
    {
        index++;
        QString id = QString::fromStdString(profileVal.value("id", ""));
        QString name = QString::fromStdString(profileVal.value("name", ""));
        bool legacy = profileVal.value("legacy", false);
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
    const auto& profile = profiles[currentProfileIndex];

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
    auto typeV = data.value("type", nlohmann::json{});
    if(!typeV.is_string()) {
        qWarning() << "Failed to parse account data: type is missing.";
        return false;
    }

    if(typeV == "MSA") {
        type = AccountType::MSA;
    } else if (typeV == "Mojang") {
        type = AccountType::Mojang;
    } else if (typeV == "Offline") {
        type = AccountType::Offline;
    } else {
        qWarning() << "Failed to parse account data: type is not recognized.";
        return false;
    }

    if(type == AccountType::Mojang) {
        legacy = data.value("legacy", false);
        canMigrateToMSA = data.value("canMigrateToMSA", false);
    }

    if(type == AccountType::MSA) {
        auto clientIDV = data.value("msa-client-id", nlohmann::json{});
        if (clientIDV.is_string()) {
            msaClientID = clientIDV.get<std::string>().c_str();
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
        return {};
    }
    return yggdrasilToken.extra["userName"].toString();
}

QString AccountData::accessToken() const {
    return yggdrasilToken.token;
}

QString AccountData::clientToken() const {
    if(type != AccountType::Mojang) {
        return {};
    }
    return yggdrasilToken.extra["clientToken"].toString();
}

void AccountData::setClientToken(const QString& clientToken) {
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
