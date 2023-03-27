#pragma once

#include "AccountData.h"
#include <nlohmann/json.hpp>

namespace Parsers
{
    bool getDateTime(const nlohmann::json& value, QDateTime& out);
    bool getString(const nlohmann::json& value, QString& out);
    bool getNumber(const nlohmann::json& value, int64_t& out);
    bool getBool(const nlohmann::json& value, bool& out);

    bool parseXTokenResponse(QByteArray& data, Katabasis::Token& output, const QString& name);
    bool parseMojangResponse(QByteArray& data, Katabasis::Token& output);

    bool parseMinecraftProfile(QByteArray& data, MinecraftProfile& output);
    bool parseMinecraftProfileMojang(QByteArray& data, MinecraftProfile& output);
    bool parseMinecraftEntitlements(QByteArray& data, MinecraftEntitlement& output);
    bool parseRolloutResponse(QByteArray& data, bool& result);
}
