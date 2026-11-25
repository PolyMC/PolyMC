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

#include "Json.h"

#include <QJsonParseError>

#include "FileSystem.h"
#include <cstring>
#include <math.h>

namespace Json
{
void write(const QJsonDocument& doc, const QString& filename)
{
    FS::write(filename, doc.toJson());
}
void write(const QJsonObject& object, const QString& filename)
{
    write(QJsonDocument(object), filename);
}
void write(const QJsonArray& array, const QString& filename)
{
    write(QJsonDocument(array), filename);
}

QByteArray toText(const QJsonObject& obj)
{
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}
QByteArray toText(const QJsonArray& array)
{
    return QJsonDocument(array).toJson(QJsonDocument::Compact);
}

static bool isBinaryJson(const QByteArray& data)
{
    decltype(QJsonDocument::BinaryFormatTag) tag =
        QJsonDocument::BinaryFormatTag;
    if (data.size() < int(sizeof(tag)))
    {
        return false;
    }
    return memcmp(data.constData(), &tag,
                  sizeof(QJsonDocument::BinaryFormatTag)) == 0;
}
QJsonDocument requireDocument(const QByteArray& data, const QString& what)
{
    if (isBinaryJson(data))
    {
        // FIXME: Is this needed?
        throw JsonException(what + ": Invalid JSON. Binary JSON unsupported");
    }
    else
    {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError)
        {
            throw JsonException(what +
                                ": Error parsing JSON: " + error.errorString());
        }
        return doc;
    }
}
QJsonDocument requireDocument(const QString& filename, const QString& what)
{
    QByteArray data;
    try
    {
        data = FS::read(filename);
    }
    catch (const FS::FileSystemException& e)
    {
        throw JsonException(e.cause());
    }
    return requireDocument(data, what);
}
QJsonObject requireObject(const QJsonDocument& doc, const QString& what)
{
    if (!doc.isObject())
    {
        throw JsonException(what + " is not an object");
    }
    return doc.object();
}
QJsonArray requireArray(const QJsonDocument& doc, const QString& what)
{
    if (!doc.isArray())
    {
        throw JsonException(what + " is not an array");
    }
    return doc.array();
}

void writeString(QJsonObject& to, const QString& key, const QString& value)
{
    if (!value.isEmpty())
    {
        to.insert(key, value);
    }
}

void writeStringList(QJsonObject& to, const QString& key,
                     const QStringList& values)
{
    if (!values.isEmpty())
    {
        QJsonArray array;
        for (const auto& value : values)
        {
            array.append(value);
        }
        to.insert(key, array);
    }
}

namespace
{
const char* convert(const QJsonValue& value, QString& out)
{
    if (!value.isString())
    {
        return " is not a string";
    }
    out = value.toString();
    return nullptr;
}

const char* convert(const QJsonValue& value, bool& out)
{
    if (!value.isBool())
    {
        return " is not a bool";
    }
    out = value.toBool();
    return nullptr;
}

const char* convert(const QJsonValue& value, double& out)
{
    if (!value.isDouble())
    {
        return " is not a double";
    }
    out = value.toDouble();
    return nullptr;
}

const char* convert(const QJsonValue& value, int& out)
{
    double real = 0.0;
    const char* complaint = convert(value, real);
    if (complaint)
    {
        return complaint;
    }
    if (fmod(real, 1) != 0)
    {
        return " is not an integer";
    }
    out = int(real);
    return nullptr;
}

const char* convert(const QJsonValue& value, QJsonObject& out)
{
    if (!value.isObject())
    {
        return " is not an object";
    }
    out = value.toObject();
    return nullptr;
}

const char* convert(const QJsonValue& value, QJsonArray& out)
{
    if (!value.isArray())
    {
        return " is not an array";
    }
    out = value.toArray();
    return nullptr;
}

const char* convert(const QJsonValue& value, QUrl& out)
{
    QString text;
    const char* complaint = convert(value, text);
    if (complaint)
    {
        return complaint;
    }
    if (text.isEmpty())
    {
        out = QUrl();
        return nullptr;
    }
    out = QUrl(text, QUrl::StrictMode);
    return out.isValid() ? nullptr : " is not a correctly formatted URL";
}

const char* convert(const QJsonValue& value, QVariant& out)
{
    if (value.isNull() || value.isUndefined())
    {
        return " is null or undefined";
    }
    out = value.toVariant();
    return nullptr;
}

QString named(const QString& what, const QString& key)
{
    return what.isEmpty() ? '\'' + key + '\'' : what;
}

template <typename T>
T require(const QJsonValue& value, const QString& what)
{
    T out{};
    const char* complaint = convert(value, out);
    if (complaint)
    {
        throw JsonException(what + complaint);
    }
    return out;
}

template <typename T>
T require(const QJsonObject& parent, const QJsonValue& value,
          const QString& key, const QString& what)
{
    if (value.isUndefined() && !parent.contains(key))
    {
        const QString label = named(what, key);
        throw JsonException(label + "s parent does not contain " + label);
    }
    T out{};
    const char* complaint = convert(value, out);
    if (complaint)
    {
        throw JsonException(named(what, key) + complaint);
    }
    return out;
}

template <typename T>
T ensure(const QJsonValue& value, const T& fallback)
{
    T out{};
    if (value.isUndefined() || value.isNull() || convert(value, out))
    {
        return fallback;
    }
    return out;
}
}  // namespace

#define JSON_HELPERFUNCTIONS(NAME, TYPE)                                       \
    TYPE require##NAME(const QJsonValue& value, const QString& what)           \
    {                                                                          \
        return require<TYPE>(value,                                            \
                             what.isEmpty() ? QStringLiteral("Value") : what); \
    }                                                                          \
    TYPE require##NAME(const QJsonObject& parent, const QString& key,          \
                       const QString& what)                                    \
    {                                                                          \
        return require<TYPE>(parent, parent.value(key), key, what);            \
    }                                                                          \
    TYPE require##NAME(const QJsonObject& parent, const char* key,             \
                       const QString& what)                                    \
    {                                                                          \
        const QJsonValue value = parent.value(QLatin1StringView(key));         \
        TYPE out{};                                                            \
        if (value.isUndefined() || convert(value, out))                        \
        {                                                                      \
            return require<TYPE>(parent, value, QString::fromLatin1(key),      \
                                 what);                                        \
        }                                                                      \
        return out;                                                            \
    }                                                                          \
    TYPE ensure##NAME(const QJsonValue& value, const TYPE& fallback)           \
    {                                                                          \
        return ensure<TYPE>(value, fallback);                                  \
    }                                                                          \
    TYPE ensure##NAME(const QJsonObject& parent, const QString& key,           \
                      const TYPE& fallback)                                    \
    {                                                                          \
        return ensure<TYPE>(parent.value(key), fallback);                      \
    }                                                                          \
    TYPE ensure##NAME(const QJsonObject& parent, const char* key,              \
                      const TYPE& fallback)                                    \
    {                                                                          \
        return ensure<TYPE>(parent.value(QLatin1StringView(key)), fallback);   \
    }

JSON_HELPERFUNCTIONS(Array, QJsonArray)
JSON_HELPERFUNCTIONS(Object, QJsonObject)
JSON_HELPERFUNCTIONS(String, QString)
JSON_HELPERFUNCTIONS(Boolean, bool)
JSON_HELPERFUNCTIONS(Double, double)
JSON_HELPERFUNCTIONS(Integer, int)
JSON_HELPERFUNCTIONS(Url, QUrl)
JSON_HELPERFUNCTIONS(Variant, QVariant)

#undef JSON_HELPERFUNCTIONS

}  // namespace Json
