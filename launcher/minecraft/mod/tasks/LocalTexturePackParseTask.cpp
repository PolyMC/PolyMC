// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
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
 */

#include "LocalTexturePackParseTask.h"

#include "FileSystem.h"

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

namespace TexturePackUtils {

bool process(TexturePack& pack)
{
    switch (pack.type()) {
        case ResourceType::FOLDER:
            TexturePackUtils::processFolder(pack);
            return true;
        case ResourceType::ZIPFILE:
            TexturePackUtils::processZIP(pack);
            return true;
        default:
            qWarning() << "Invalid type for resource pack parse task!";
            return false;
    }
}

void processFolder(TexturePack& pack)
{
    Q_ASSERT(pack.type() == ResourceType::FOLDER);

    // some old texture packs might use other schemas, so fallback to those
    static const QStringList candidates = {
        "pack.txt",
        "pack.json",
        "manifest.json",
        "manifest.json.txt"
    };

    for (const QString& c : candidates) {
        QFileInfo mcmeta_file_info(FS::PathCombine(pack.fileinfo().filePath(), c));
        if (!mcmeta_file_info.isFile())
            continue;

        QFile mcmeta_file(mcmeta_file_info.filePath());

        if (!mcmeta_file.open(QIODevice::ReadOnly))
            return;

        auto data = mcmeta_file.readAll();
        mcmeta_file.close();
        if (data.isEmpty())
            continue;

        TexturePackUtils::processPackTXT(pack, std::move(data));
        break;
    }

    QFileInfo image_file_info(FS::PathCombine(pack.fileinfo().filePath(), "pack.png"));
    if (image_file_info.isFile()) {
        QFile mcmeta_file(image_file_info.filePath());
        if (!mcmeta_file.open(QIODevice::ReadOnly))
            return;

        auto data = mcmeta_file.readAll();

        TexturePackUtils::processPackPNG(pack, std::move(data));

        mcmeta_file.close();
    }
}

void processZIP(TexturePack& pack)
{
    Q_ASSERT(pack.type() == ResourceType::ZIPFILE);

    QuaZip zip(pack.fileinfo().filePath());
    if (!zip.open(QuaZip::mdUnzip))
        return;

    QuaZipFile file(&zip);

    // some old texture packs might use other schemas, so fallback to those
    static const QStringList candidates = {
        "pack.txt",
        "pack.json",
        "manifest.json",
        "manifest.json.txt"
    };

    for (const QString& c : candidates) {
        if (!zip.setCurrentFile(c))
            continue;

        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Failed to open file in zip.";
            zip.close();
            return;
        }

        auto data = file.readAll();
        file.close();

        if (data.isEmpty())
            continue;

        TexturePackUtils::processPackTXT(pack, std::move(data));
        break;
    }

    if (zip.setCurrentFile("pack.png")) {
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Failed to open file in zip.";
            zip.close();
            return;
        }

        auto data = file.readAll();

        TexturePackUtils::processPackPNG(pack, std::move(data));

        file.close();
    }

    zip.close();
}

void processPackTXT(TexturePack& pack, QByteArray&& raw_data)
{
    // some packs may have a JSON pack.txt, or none at all
    // in this case fall back to manifest.json parsing
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(raw_data, &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        auto obj = doc.object().value("description").isObject()
        ? doc.object().value("description").toObject()
        : doc.object();
        QStringList parts = {
            obj.value("name").toString(),
            obj.value("line1").toString(),
            obj.value("line2").toString()
        };
        parts.removeAll({});
        if (!parts.isEmpty()) {
            pack.setDescription(parts.join('\n'));
            return;
        }
    }

    pack.setDescription(QString(raw_data).trimmed());
}

void processPackPNG(TexturePack& pack, QByteArray&& raw_data)
{
    auto img = QImage::fromData(raw_data);
    if (!img.isNull()) {
        pack.setImage(img);
    } else {
        qWarning() << "Failed to parse pack.png.";
    }
}
}  // namespace TexturePackUtils

LocalTexturePackParseTask::LocalTexturePackParseTask(int token, TexturePack& rp)
    : Task(nullptr, false), m_token(token), m_texture_pack(rp)
{}

bool LocalTexturePackParseTask::abort()
{
    m_aborted = true;
    return true;
}

void LocalTexturePackParseTask::executeTask()
{
    Q_ASSERT(m_texture_pack.valid());

    if (!TexturePackUtils::process(m_texture_pack))
        return;

    if (m_aborted)
        emitAborted();
    else
        emitSucceeded();
}
