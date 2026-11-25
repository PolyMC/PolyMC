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

#pragma once

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>

#include "Exception.h"

namespace Json
{
class JsonException : public ::Exception
{
public:
    JsonException(const QString& message) : Exception(message)
    {
    }
};

/// @throw FileSystemException
void write(const QJsonDocument& doc, const QString& filename);
/// @throw FileSystemException
void write(const QJsonObject& object, const QString& filename);
/// @throw FileSystemException
void write(const QJsonArray& array, const QString& filename);

QByteArray toText(const QJsonObject& obj);
QByteArray toText(const QJsonArray& array);

/// @throw JsonException
QJsonDocument requireDocument(const QByteArray& data,
                              const QString& what = "Document");
/// @throw JsonException
QJsonDocument requireDocument(const QString& filename,
                              const QString& what = "Document");
/// @throw JsonException
QJsonObject requireObject(const QJsonDocument& doc,
                          const QString& what = "Document");
/// @throw JsonException
QJsonArray requireArray(const QJsonDocument& doc,
                        const QString& what = "Document");

/////////////////// WRITING ////////////////////

void writeString(QJsonObject& to, const QString& key, const QString& value);
void writeStringList(QJsonObject& to, const QString& key,
                     const QStringList& values);

////////////////// READING ////////////////////

#define JSON_HELPERFUNCTIONS(NAME, TYPE)                                   \
    /** @throw JsonException */                                            \
    TYPE require##NAME(const QJsonValue& value, const QString& what = {}); \
    /** @throw JsonException */                                            \
    TYPE require##NAME(const QJsonObject& parent, const QString& key,      \
                       const QString& what = {});                          \
    /** @throw JsonException */                                            \
    TYPE require##NAME(const QJsonObject& parent, const char* key,         \
                       const QString& what = {});                          \
    TYPE ensure##NAME(const QJsonValue& value, const TYPE& fallback = {}); \
    TYPE ensure##NAME(const QJsonObject& parent, const QString& key,       \
                      const TYPE& fallback = {});                          \
    TYPE ensure##NAME(const QJsonObject& parent, const char* key,          \
                      const TYPE& fallback = {});

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
using JSONValidationError = Json::JsonException;
