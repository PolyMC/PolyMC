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

#include "ListModel.h"

#include <MMCStrings.h>
#include <Version.h>

#include <QColor>
#include <QDebug>

#include <BuildConfig.h>

#include "ui/pages/modplatform/LogoCache.h"

namespace LegacyFTB
{

namespace
{
QString logoUrl(const QString& logo)
{
    return (BuildConfig.LEGACY_FTB_CDN_BASE_URL + "static/%1").arg(logo);
}
}  // namespace

FilterModel::FilterModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    currentSorting = Sorting::ByGameVersion;
    sortings.insert(tr("Sort by Name"), Sorting::ByName);
    sortings.insert(tr("Sort by Game Version"), Sorting::ByGameVersion);
}

bool FilterModel::lessThan(const QModelIndex& left,
                           const QModelIndex& right) const
{
    Modpack leftPack = sourceModel()->data(left, Qt::UserRole).value<Modpack>();
    Modpack rightPack =
        sourceModel()->data(right, Qt::UserRole).value<Modpack>();

    if (currentSorting == Sorting::ByGameVersion)
    {
        Version lv(leftPack.mcVersion);
        Version rv(rightPack.mcVersion);
        return lv < rv;
    }
    else if (currentSorting == Sorting::ByName)
    {
        return Strings::naturalCompare(leftPack.name, rightPack.name,
                                       Qt::CaseSensitive) > 0;
    }

    // UHM, some inavlid value set?!
    qWarning() << "Invalid sorting set!";
    return true;
}

bool FilterModel::filterAcceptsRow(int /*sourceRow*/,
                                   const QModelIndex& /*sourceParent*/) const
{
    return true;
}

const QMap<QString, FilterModel::Sorting> FilterModel::getAvailableSortings()
{
    return sortings;
}

QString FilterModel::translateCurrentSorting()
{
    return sortings.key(currentSorting);
}

void FilterModel::setSorting(Sorting s)
{
    currentSorting = s;
    invalidate();
}

FilterModel::Sorting FilterModel::getCurrentSorting()
{
    return currentSorting;
}

ListModel::ListModel(QObject* parent)
    : QAbstractListModel(parent),
      m_logos(new LogoCache("FTBPacks", "FTB Icon Download for",
                            "screenshot-placeholder", LogoCache::Skip::Nothing,
                            this))
{
    connect(m_logos, &LogoCache::logoLoaded, this, &ListModel::onLogoLoaded);
}

ListModel::~ListModel()
{
}

QString ListModel::translatePackType(PackType type) const
{
    switch (type)
    {
        case PackType::Public:
            return tr("Public Modpack");
        case PackType::ThirdParty:
            return tr("Third Party Modpack");
        case PackType::Private:
            return tr("Private Modpack");
    }
    qWarning() << "Unknown FTB modpack type:" << int(type);
    return QString();
}

int ListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return modpacks.size();
}

int ListModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant ListModel::data(const QModelIndex& index, int role) const
{
    int pos = index.row();
    if (pos >= modpacks.size() || pos < 0 || !index.isValid())
    {
        return QString("INVALID INDEX %1").arg(pos);
    }

    Modpack pack = modpacks.at(pos);
    if (role == Qt::DisplayRole)
    {
        return pack.name + "\n" + translatePackType(pack.type);
    }
    else if (role == Qt::ToolTipRole)
    {
        if (pack.description.length() > 100)
        {
            // some magic to prevent to long tooltips and replace html
            // linebreaks
            QString edit = pack.description.left(97);
            edit = edit.left(edit.lastIndexOf("<br>"))
                       .left(edit.lastIndexOf(" "))
                       .append("...");
            return edit;
        }
        return pack.description;
    }
    else if (role == Qt::DecorationRole)
    {
        return m_logos->icon(pack.logo, logoUrl(pack.logo));
    }
    else if (role == Qt::ForegroundRole)
    {
        if (pack.broken)
        {
            // FIXME: Hardcoded color
            return QColor(255, 0, 50);
        }
        else if (pack.bugged)
        {
            // FIXME: Hardcoded color
            // bugged pack, currently only indicates bugged xml
            return QColor(244, 229, 66);
        }
    }
    else if (role == Qt::UserRole)
    {
        QVariant v;
        v.setValue(pack);
        return v;
    }

    return QVariant();
}

void ListModel::fill(ModpackList newModpacks)
{
    beginResetModel();
    this->modpacks = newModpacks;
    endResetModel();
}

void ListModel::addPack(const Modpack& modpack)
{
    beginResetModel();
    this->modpacks.append(modpack);
    endResetModel();
}

void ListModel::clear()
{
    beginResetModel();
    modpacks.clear();
    endResetModel();
}

Modpack ListModel::at(int row)
{
    return modpacks.at(row);
}

void ListModel::remove(int row)
{
    if (row < 0 || row >= modpacks.size())
    {
        qWarning() << "Attempt to remove FTB modpacks with invalid row" << row;
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    modpacks.removeAt(row);
    endRemoveRows();
}

void ListModel::onLogoLoaded()
{
    // Any of the packs may use this logo, not just the first two
    if (!modpacks.isEmpty())
    {
        emit dataChanged(createIndex(0, 0),
                         createIndex(modpacks.size() - 1, 0));
    }
}

QString ListModel::logoPath(const QString& logo)
{
    return m_logos->path(logo, logoUrl(logo));
}

Qt::ItemFlags ListModel::flags(const QModelIndex& index) const
{
    return QAbstractListModel::flags(index);
}

}  // namespace LegacyFTB

#include "moc_ListModel.cpp"
