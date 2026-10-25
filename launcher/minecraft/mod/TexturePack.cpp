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

#include "TexturePack.h"

#include <QDebug>
#include <QMap>
#include <QRegularExpression>

#include "minecraft/mod/tasks/LocalTexturePackParseTask.h"

void TexturePack::setDescription(QString new_description)
{
    QMutexLocker locker(&m_data_lock);

    m_description = new_description;
}

void TexturePack::setImage(QImage new_image)
{
    QMutexLocker locker(&m_data_lock);

    Q_ASSERT(!new_image.isNull());

    m_pack_image = new_image;
    m_pack_image_cache_key = QPixmapCache::Key();
}

QPixmap TexturePack::image(QSize size)
{
    QMutexLocker locker(&m_data_lock);

    QPixmap cached_image;
    if (!QPixmapCache::find(m_pack_image_cache_key, &cached_image)) {
        if (m_pack_image.isNull())
            return {};
        cached_image = QPixmap::fromImage(m_pack_image);
        m_pack_image_cache_key = QPixmapCache::insert(cached_image);
    }

    if (size.isNull())
        return cached_image;
    return cached_image.scaled(size);
}
