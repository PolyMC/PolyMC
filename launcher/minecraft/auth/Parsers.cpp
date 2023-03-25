#include "Parsers.h"

#include <QDebug>

#include <nlohmann/json.hpp>

namespace Parsers {

bool getDateTime(const nlohmann::json& value, QDateTime& out) {
    if(!value.is_string())
        return false;

    out = QDateTime::fromString(value.get<std::string>().c_str(), Qt::ISODate);
    return out.isValid();
}

bool getString(const nlohmann::json& value, QString& out) {
    if(!value.is_string())
        return false;

    out = value.get<std::string>().c_str();
    return true;
}

bool getNumber(const nlohmann::json& value, int64_t& out) {
    if(!value.is_number_integer())
        return false;

    out = value.get<int64_t>();
    return true;
}

bool getBool(const nlohmann::json& value, bool& out) {
    if(!value.is_boolean())
        return false;

    out = value.get<bool>();
    return true;
}

/*
{
   "IssueInstant":"2020-12-07T19:52:08.4463796Z",
   "NotAfter":"2020-12-21T19:52:08.4463796Z",
   "Token":"token",
   "DisplayClaims":{
      "xui":[
         {
            "uhs":"userhash"
         }
      ]
   }
 }
*/
// TODO: handle error responses ...
/*
{
    "Identity":"0",
    "XErr":2148916238,
    "Message":"",
    "Redirect":"https://start.ui.xboxlive.com/AddChildToFamily"
}
// 2148916233 = missing XBox account
// 2148916238 = child account not linked to a family
*/

bool parseXTokenResponse(QByteArray & data, Katabasis::Token &output, const QString& name) {
    qDebug() << "Parsing" << name <<":";
#ifndef NDEBUG
    qDebug() << data;
#endif
    nlohmann::json obj;
    try {
        obj = nlohmann::json::parse(data.constData(), data.constData() + data.size());
    } catch (nlohmann::json::parse_error& e) {
        qWarning() << "Failed to parse response from user.auth.xboxlive.com as JSON: " << e.what();
        return false;
    }

    //the function calls here are probably unnecessary
    if(!getDateTime(obj.value("IssueInstant", nlohmann::json()), output.issueInstant)) {
        qWarning() << "User IssueInstant is not a timestamp";
        return false;
    }

    if(!getDateTime(obj.value("NotAfter", nlohmann::json()), output.notAfter)) {
        qWarning() << "User NotAfter is not a timestamp";
        return false;
    }

    if(!getString(obj.value("Token", nlohmann::json()), output.token)) {
        qWarning() << "User Token is not a string";
        return false;
    }
    //auto arrayVal = obj.value("DisplayClaims").toObject().value("xui");
    auto arrayVal = obj.value("DisplayClaims", nlohmann::json()).value("xui", nlohmann::json());
    if(!arrayVal.is_array()) {
        qWarning() << "Missing xui claims array";
        return false;
    }
    bool foundUHS = false;
    //for(auto item: arrayVal.toArray()) {
    for(const auto& arrObj: arrayVal) {
        if(!arrObj.is_object()) {
            continue;
        }
        if(arrObj.contains("uhs")) {
            foundUHS = true;
        } else {
            continue;
        }
        // consume all 'display claims' ... whatever that means
        /*
        for(auto iter = obj.begin(); iter != obj.end(); iter++) {
            QString claim;
            if(!getString(obj.value(iter.key()), claim)) {
                qWarning() << "display claim " << iter.key() << " is not a string...";
                return false;
            }
            output.extra[iter.key()] = claim;
        }
         */
        for (auto it = arrObj.begin(); it != arrObj.end(); ++it) {
            QString claim;
            if(!getString(it.value(), claim)) {
                qWarning() << "display claim " << it.key().c_str() << " is not a string...";
                return false;
            }
            output.extra[it.key().c_str()] = claim;
        }


        break;
    }
    if(!foundUHS) {
        qWarning() << "Missing uhs";
        return false;
    }
    output.validity = Katabasis::Validity::Certain;
    qDebug() << name << "is valid.";
    return true;
}

bool parseMinecraftProfile(QByteArray& data, MinecraftProfile& output) {
    qDebug() << "Parsing Minecraft profile...";
#ifndef NDEBUG
    qDebug() << data;
#endif

    nlohmann::json obj;
    try {
        obj = nlohmann::json::parse(data.constData(), data.constData() + data.size());
    } catch (nlohmann::json::parse_error& e) {
        qWarning() << "Failed to parse response from user.auth.xboxlive.com as JSON: " << e.what();
        return false;
    }


    if(!getString(obj.value("id", nlohmann::json()), output.id)) {
        qWarning() << "Minecraft profile id is not a string";
        return false;
    }

    if(!getString(obj.value("name", nlohmann::json()), output.name)) {
        qWarning() << "Minecraft profile name is not a string";
        return false;
    }

    //auto skinsArray = obj.value("skins").toArray();
    auto skinsArray = obj.value("skins", nlohmann::json());
    for(const auto& skinObj: skinsArray) {
        Skin skinOut;
        if(!getString(skinObj.value("id", nlohmann::json()), skinOut.id)) {
            continue;
        }
        QString state;
        if(!getString(skinObj.value("state", nlohmann::json()), state)) {
            continue;
        }
        if(state != "ACTIVE") {
            continue;
        }
        if(!getString(skinObj.value("url", nlohmann::json()), skinOut.url)) {
            continue;
        }
        if(!getString(skinObj.value("variant", nlohmann::json()), skinOut.variant)) {
            continue;
        }
        // we deal with only the active skin
        output.skin = skinOut;
        break;
    }

    auto capesArray = obj.value("capes", nlohmann::json());
    QString currentCape;
    for(const auto& capeObj: capesArray) {
        Cape capeOut;
        if(!getString(capeObj.value("id", nlohmann::json()), capeOut.id)) {
            continue;
        }
        QString state;
        if(!getString(capeObj.value("state", nlohmann::json()), state)) {
            continue;
        }
        if(state == "ACTIVE") {
            currentCape = capeOut.id;
        }
        if(!getString(capeObj.value("url", nlohmann::json()), capeOut.url)) {
            continue;
        }
        if(!getString(capeObj.value("alias", nlohmann::json()), capeOut.alias)) {
            continue;
        }

        output.capes[capeOut.id] = capeOut;
    }
    output.currentCape = currentCape;
    output.validity = Katabasis::Validity::Certain;
    return true;
}

namespace {
    // these skin URLs are for the MHF_Steve and MHF_Alex accounts (made by a Mojang employee)
    // they are needed because the session server doesn't return skin urls for default skins
    const QString SKIN_URL_STEVE = "https://textures.minecraft.net/texture/1a4af718455d4aab528e7a61f86fa25e6a369d1768dcb13f7df319a713eb810b";
    const QString SKIN_URL_ALEX = "https://textures.minecraft.net/texture/83cee5ca6afcdb171285aa00e8049c297b2dbeba0efb8ff970a5677a1b644032";

    bool isDefaultModelSteve(QString uuid) {
        // need to calculate *Java* hashCode of UUID
        // if number is even, skin/model is steve, otherwise it is alex

        // just in case dashes are in the id
        uuid.remove('-');

        if (uuid.size() != 32) {
            return true;
        }

        // qulonglong is guaranteed to be 64 bits
        // we need to use unsigned numbers to guarantee truncation below
        qulonglong most = uuid.left(16).toULongLong(nullptr, 16);
        qulonglong least = uuid.right(16).toULongLong(nullptr, 16);
        qulonglong xored = most ^ least;
        return ((static_cast<quint32>(xored >> 32)) ^ static_cast<quint32>(xored)) % 2 == 0;
    }
}

/**
Uses session server for skin/cape lookup instead of profile,
because locked Mojang accounts cannot access profile endpoint
(https://api.minecraftservices.com/minecraft/profile/)

ref: https://wiki.vg/Mojang_API#UUID_to_Profile_and_Skin.2FCape

{
    "id": "<profile identifier>",
    "name": "<player name>",
    "properties": [
        {
            "name": "textures",
            "value": "<base64 string>"
        }
    ]
}

decoded base64 "value":
{
    "timestamp": <java time in ms>,
    "profileId": "<profile uuid>",
    "profileName": "<player name>",
    "textures": {
        "SKIN": {
            "url": "<player skin URL>"
        },
        "CAPE": {
            "url": "<player cape URL>"
        }
    }
}
*/

bool parseMinecraftProfileMojang(QByteArray& data, MinecraftProfile& output) {
    qDebug() << "Parsing Minecraft profile...";
#ifndef NDEBUG
    qDebug() << data;
#endif

    nlohmann::json obj;
    try {
        obj = nlohmann::json::parse(data.constData(), data.constData() + data.size());
    } catch (nlohmann::json::parse_error &e) {
        qWarning() << "Failed to parse response as JSON: " << e.what();
        return false;
    }

    if(!getString(obj.value("id", nlohmann::json()), output.id)) {
        qWarning() << "Minecraft profile id is not a string";
        return false;
    }

    if(!getString(obj.value("name", nlohmann::json()), output.name)) {
        qWarning() << "Minecraft profile name is not a string";
        return false;
    }

    auto propsArray = obj.value("properties", nlohmann::json());
    QByteArray texturePayload;
    for (const auto& pObj : propsArray) {
        auto name = pObj.value("name", nlohmann::json());
        if (!name.is_string() || name.get<std::string>() != "textures") {
            continue;
        }

        auto value = pObj.value("value", nlohmann::json());
        if (value.is_string()) {
            QString valueStr = value.get<std::string>().c_str();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            texturePayload = QByteArray::fromBase64(valueStr.toUtf8(), QByteArray::AbortOnBase64DecodingErrors);
#else
            texturePayload = QByteArray::fromBase64(valueStr.toUtf8());
#endif
        }

        if (!texturePayload.isEmpty()) {
            break;
        }
    }

    if (texturePayload.isNull()) {
        qWarning() << "No texture payload data";
        return false;
    }

    try {
        obj = nlohmann::json::parse(texturePayload.constData(), texturePayload.constData() + texturePayload.size());
    } catch (nlohmann::json::parse_error &e) {
        qWarning() << "Failed to parse response as JSON: " << e.what();
        return false;
    }

    auto textures = obj.value("textures", nlohmann::json());
    if (!textures.is_object()) {
        qWarning() << "No textures array in response";
        return false;
    }

    Skin skinOut;
    // fill in default skin info ourselves, as this endpoint doesn't provide it
    bool steve = isDefaultModelSteve(output.id);
    skinOut.variant = steve ? "classic" : "slim";
    skinOut.url = steve ? SKIN_URL_STEVE : SKIN_URL_ALEX;
    // sadly we can't figure this out, but I don't think it really matters...
    skinOut.id = "00000000-0000-0000-0000-000000000000";
    Cape capeOut;
    //auto tObj = textures.toObject();
    //for (auto idx = tObj.constBegin(); idx != tObj.constEnd(); ++idx) {
    for (auto idx = textures.begin(); idx != textures.end(); ++idx) {
        if (idx->is_object()) {
            if (idx.key() == "SKIN") {
                //auto skin = idx->toObject();
                nlohmann::json skin = *idx;
                if (!getString(skin.value("url", nlohmann::json()), skinOut.url)) {
                    qWarning() << "Skin url is not a string";
                    return false;
                }

                auto maybeMeta = skin.find("metadata");
                if (maybeMeta != skin.end() && maybeMeta->is_object()) {
                    nlohmann::json meta = *maybeMeta;
                    // might not be present
                    getString(meta.value("model", nlohmann::json()), skinOut.variant);
                }
            }
            else if (idx.key() == "CAPE") {
                //auto cape = idx->toObject();
                nlohmann::json cape = *idx;
                if (!getString(cape.value("url", nlohmann::json()), capeOut.url)) {
                    qWarning() << "Cape url is not a string";
                    return false;
                }

                // we don't know the cape ID as it is not returned from the session server
                // so just fake it - changing capes is probably locked anyway :(
                capeOut.alias = "cape";
            }
        }
    }

    output.skin = skinOut;
    if (capeOut.alias == "cape") {
        output.capes = QMap<QString, Cape>({{capeOut.alias, capeOut}});
        output.currentCape = capeOut.alias;
    }

    output.validity = Katabasis::Validity::Certain;
    return true;
}

bool parseMinecraftEntitlements(QByteArray& data, MinecraftEntitlement& output) {
    qDebug() << "Parsing Minecraft entitlements...";
#ifndef NDEBUG
    qDebug() << data;
#endif
    nlohmann::json obj;
    try {
        obj = nlohmann::json::parse(data.constData(), data.constData() + data.size());
    } catch (nlohmann::json::parse_error &e) {
        qWarning() << "Failed to parse response as JSON: " << e.what();
        return false;
    }

    output.canPlayMinecraft = false;
    output.ownsMinecraft = false;

    auto itemsArray = obj.value("items", nlohmann::json());
    for(const auto& itemObj: itemsArray) {
        QString name;
        if(!getString(itemObj.value("name", nlohmann::json()), name)) {
            continue;
        }
        if(name == "game_minecraft") {
            output.canPlayMinecraft = true;
        }
        if(name == "product_minecraft") {
            output.ownsMinecraft = true;
        }
    }
    output.validity = Katabasis::Validity::Certain;
    return true;
}

bool parseRolloutResponse(QByteArray & data, bool& result) {
    qDebug() << "Parsing Rollout response...";
#ifndef NDEBUG
    qDebug() << data;
#endif
    nlohmann::json obj;
    try {
        obj = nlohmann::json::parse(data.constData(), data.constData() + data.size());
    } catch (nlohmann::json::parse_error &e) {
        qWarning() << "Failed to parse response as JSON: " << e.what();
        return false;
    }

    QString feature;
    if(!getString(obj.value("feature", nlohmann::json()), feature)) {
        qWarning() << "Rollout feature is not a string";
        return false;
    }
    if(feature != "msamigration") {
        qWarning() << "Rollout feature is not what we expected (msamigration), but is instead \"" << feature << "\"";
        return false;
    }
    if(!getBool(obj.value("rollout", nlohmann::json()), result)) {
        qWarning() << "Rollout feature is not a string";
        return false;
    }
    return true;
}

bool parseMojangResponse(QByteArray & data, Katabasis::Token &output) {
    qDebug() << "Parsing Mojang response...";
#ifndef NDEBUG
    qDebug() << data;
#endif
    nlohmann::json obj;
    try {
        obj = nlohmann::json::parse(data.constData(), data.constData() + data.size());
    } catch (nlohmann::json::parse_error &e) {
        qWarning() << "Failed to parse response as JSON: " << e.what();
        return false;
    }

    int64_t expires_in = 0;
    if(!getNumber(obj.value("expires_in", nlohmann::json()), expires_in)) {
        qWarning() << "expires_in is not a valid number";
        return false;
    }
    auto currentTime = QDateTime::currentDateTimeUtc();
    output.issueInstant = currentTime;
    output.notAfter = currentTime.addSecs(expires_in);

    QString username;
    if(!getString(obj.value("username", nlohmann::json()), username)) {
        qWarning() << "username is not valid";
        return false;
    }

    // TODO: it's a JWT... validate it?
    if(!getString(obj.value("access_token", nlohmann::json()), output.token)) {
        qWarning() << "access_token is not valid";
        return false;
    }
    output.validity = Katabasis::Validity::Certain;
    qDebug() << "Mojang response is valid.";
    return true;
}

}
