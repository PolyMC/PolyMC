// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
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
 */

#include "LocalResourcePackParseTask.h"

#include "FileSystem.h"
#include "Json.h"

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include <QCryptographicHash>

namespace ResourcePackUtils {

bool process(ResourcePack& pack)
{
    switch (pack.type()) {
        case ResourceType::FOLDER:
            ResourcePackUtils::processFolder(pack);
            return true;
        case ResourceType::ZIPFILE:
            ResourcePackUtils::processZIP(pack);
            return true;
        default:
            qWarning() << "Invalid type for resource pack parse task!";
            return false;
    }
}

void processFolder(ResourcePack& pack)
{
    Q_ASSERT(pack.type() == ResourceType::FOLDER);

    QFileInfo mcmeta_file_info(FS::PathCombine(pack.fileinfo().filePath(), "pack.mcmeta"));
    if (mcmeta_file_info.isFile()) {
        QFile mcmeta_file(mcmeta_file_info.filePath());
        if (!mcmeta_file.open(QIODevice::ReadOnly))
            return;

        auto data = mcmeta_file.readAll();

        ResourcePackUtils::processMCMeta(pack, std::move(data));

        mcmeta_file.close();
    }

    QFileInfo image_file_info(FS::PathCombine(pack.fileinfo().filePath(), "pack.png"));
    if (image_file_info.isFile()) {
        QFile mcmeta_file(image_file_info.filePath());
        if (!mcmeta_file.open(QIODevice::ReadOnly))
            return;

        auto data = mcmeta_file.readAll();

        ResourcePackUtils::processPackPNG(pack, std::move(data));

        mcmeta_file.close();
    }
}

void processZIP(ResourcePack& pack)
{
    Q_ASSERT(pack.type() == ResourceType::ZIPFILE);

    QuaZip zip(pack.fileinfo().filePath());
    if (!zip.open(QuaZip::mdUnzip))
        return;

    QuaZipFile file(&zip);

    if (zip.setCurrentFile("pack.mcmeta")) {
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Failed to open file in zip.";
            zip.close();
            return;
        }

        auto data = file.readAll();

        ResourcePackUtils::processMCMeta(pack, std::move(data));

        file.close();
    }

    if (zip.setCurrentFile("pack.png")) {
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Failed to open file in zip.";
            zip.close();
            return;
        }

        auto data = file.readAll();

        ResourcePackUtils::processPackPNG(pack, std::move(data));

        file.close();
    }

    zip.close();
}

// this is either a single major version, or [major, minor]
static std::pair<int, int> parsePackFormatVersion(const QJsonValue& value)
{
    if (value.isDouble())
        return { value.toInt(), 0 };

    std::pair<int, int> version;
    if (value.isArray()) {
        const auto array = value.toArray();
        if (array.size() >= 1)
            version.first = array.at(0).toInt();
        if (array.size() >= 2)
            version.second = array.at(1).toInt();
    }
    return version;
}

static QString parsePackDescription(const QJsonValue &value) {
    if (value.isString())
        return value.toString();

    if (value.isObject())
        return Json::ensureString(value.toObject().value("fallback"), "");

    return "";
}

// https://minecraft.wiki/w/Tutorials/Creating_a_resource_pack#Formatting_pack.mcmeta
void processMCMeta(ResourcePack& pack, QByteArray&& raw_data)
{
    try {
        auto json_doc = QJsonDocument::fromJson(raw_data);
        auto pack_obj = Json::requireObject(json_doc.object(), "pack", {});

        pack.setDescription(parsePackDescription(pack_obj.value("description")));

        int packFormat = 0;
        std::pair<int, int> minFormat;
        std::pair<int, int> maxFormat;
        if (pack_obj.contains("pack_format"))
            packFormat = Json::ensureInteger(pack_obj, "pack_format", 0);
        if (pack_obj.contains("min_format"))
            minFormat = parsePackFormatVersion(pack_obj.value("min_format"));
        if (pack_obj.contains("max_format"))
            maxFormat = parsePackFormatVersion(pack_obj.value("max_format"));

        pack.setPackFormat(packFormat, minFormat, maxFormat);
    } catch (Json::JsonException& e) {
        qWarning() << "JsonException: " << e.what() << e.cause();
    }
}

void processPackPNG(ResourcePack& pack, QByteArray&& raw_data)
{
    auto img = QImage::fromData(raw_data);
    if (!img.isNull()) {
        pack.setImage(img);
    } else {
        qWarning() << "Failed to parse pack.png.";
    }
}
}  // namespace ResourcePackUtils

LocalResourcePackParseTask::LocalResourcePackParseTask(int token, ResourcePack& rp)
    : Task(nullptr, false), m_token(token), m_resource_pack(rp)
{}

bool LocalResourcePackParseTask::abort()
{
    m_aborted = true;
    return true;
}

void LocalResourcePackParseTask::executeTask()
{
    Q_ASSERT(m_resource_pack.valid());

    if (!ResourcePackUtils::process(m_resource_pack))
        return;

    if (m_aborted)
        emitAborted();
    else
        emitSucceeded();
}
