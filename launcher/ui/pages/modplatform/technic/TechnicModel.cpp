// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2021 Jamie Mansfield <jmansfield@cadixdev.org>
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
 *      Copyright 2020-2021 MultiMC Contributors
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

#include "TechnicModel.h"
#include "Application.h"
#include "BuildConfig.h"
#include "Json.h"
#include "net/Download.h"

#include <QDebug>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QUrl>

#include <utility>

#include "ui/pages/modplatform/LogoCache.h"

Technic::ListModel::ListModel(QObject* parent)
    : QAbstractListModel(parent),
      m_logos(new LogoCache("TechnicPacks", "Technic Icon Download",
                            "screenshot-placeholder", LogoCache::Skip::NullName,
                            this))
{
    connect(m_logos, &LogoCache::logoLoaded, this, &ListModel::onLogoLoaded);
}

Technic::ListModel::~ListModel()
{
    if (jobPtr && jobPtr->isRunning())
    {
        jobPtr->disconnect(this);
        jobPtr->abort();
    }
}

QVariant Technic::ListModel::data(const QModelIndex& index, int role) const
{
    int pos = index.row();
    if (pos >= modpacks.size() || pos < 0 || !index.isValid())
    {
        return QString("INVALID INDEX %1").arg(pos);
    }

    Modpack pack = modpacks.at(pos);
    if (role == Qt::DisplayRole)
    {
        return pack.name;
    }
    else if (role == Qt::DecorationRole)
    {
        return m_logos->icon(pack.logoName, pack.logoUrl);
    }
    else if (role == Qt::UserRole)
    {
        QVariant v;
        v.setValue(pack);
        return v;
    }
    return QVariant();
}

int Technic::ListModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int Technic::ListModel::rowCount(const QModelIndex&) const
{
    return modpacks.size();
}

void Technic::ListModel::searchWithTerm(const QString& term)
{
    if (currentSearchTerm == term &&
        currentSearchTerm.isNull() == term.isNull())
    {
        return;
    }
    currentSearchTerm = term;
    // Only abort a search that is still going, a finished one never reports
    // back. Set the state first, aborting can report back right away.
    if (jobPtr && jobPtr->isRunning())
    {
        searchState = ResetRequested;
        jobPtr->abort();
        return;
    }
    else
    {
        beginResetModel();
        modpacks.clear();
        endResetModel();
        searchState = None;
    }
    performSearch();
}

void Technic::ListModel::performSearch()
{
    NetJob* netJob = new NetJob("Technic::Search", APPLICATION->network());
    QString searchUrl = "";
    if (currentSearchTerm.isEmpty())
    {
        searchUrl = QString("%1trending?build=%2")
                        .arg(BuildConfig.TECHNIC_API_BASE_URL,
                             BuildConfig.TECHNIC_API_BUILD);
        searchMode = List;
    }
    else if (currentSearchTerm.startsWith(
                 "http://api.technicpack.net/modpack/"))
    {
        searchUrl =
            QString("https://%1?build=%2")
                .arg(currentSearchTerm.mid(7), BuildConfig.TECHNIC_API_BUILD);
        searchMode = Single;
    }
    else if (currentSearchTerm.startsWith(
                 "https://api.technicpack.net/modpack/"))
    {
        searchUrl = QString("%1?build=%2")
                        .arg(currentSearchTerm, BuildConfig.TECHNIC_API_BUILD);
        searchMode = Single;
    }
    else
    {
        searchUrl = QString("%1search?build=%2&q=%3"
                            // Encode the search, characters like & or # would
                            // otherwise end the parameter early
                            )
                        .arg(BuildConfig.TECHNIC_API_BASE_URL,
                             BuildConfig.TECHNIC_API_BUILD,
                             QString::fromUtf8(
                                 QUrl::toPercentEncoding(currentSearchTerm)));
        searchMode = List;
    }
    netJob->addNetAction(
        Net::Download::makeByteArray(QUrl(searchUrl), &response));
    jobPtr = netJob;
    jobPtr->start();
    QObject::connect(netJob, &NetJob::succeeded, this,
                     &ListModel::searchRequestFinished);
    QObject::connect(netJob, &NetJob::failed, this,
                     &ListModel::searchRequestFailed);
    QObject::connect(netJob, &NetJob::aborted, this,
                     &ListModel::searchRequestFailed);
}

void Technic::ListModel::searchRequestFinished()
{
    jobPtr.reset();

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from Technic at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << response;
        return;
    }

    QList<Modpack> newList;
    try
    {
        auto root = Json::requireObject(doc);

        switch (searchMode)
        {
            case List:
            {
                auto objs = Json::requireArray(root, "modpacks");
                for (auto technicPack : std::as_const(objs))
                {
                    Modpack pack;
                    auto technicPackObject = Json::requireObject(technicPack);
                    pack.name = Json::requireString(technicPackObject, "name");
                    pack.slug = Json::requireString(technicPackObject, "slug");
                    if (pack.slug == "vanilla")
                    {
                        continue;
                    }

                    auto rawURL = Json::ensureString(technicPackObject,
                                                     "iconUrl", "null");
                    if (rawURL == "null")
                    {
                        pack.logoUrl = "null";
                        pack.logoName = "null";
                    }
                    else
                    {
                        pack.logoUrl = rawURL;
                        pack.logoName = rawURL.section(QLatin1Char('/'), -1)
                                            .section(QLatin1Char('.'), 0, 0);
                    }
                    pack.broken = false;
                    newList.append(pack);
                }
                break;
            }
            case Single:
            {
                if (root.contains("error"))
                {
                    // Invalid API url
                    break;
                }

                Modpack pack;
                pack.name = Json::requireString(root, "displayName");
                pack.slug = Json::requireString(root, "name");

                if (root.contains("icon"))
                {
                    auto iconObj = Json::requireObject(root, "icon");
                    auto iconUrl = Json::requireString(iconObj, "url");

                    pack.logoUrl = iconUrl;
                    pack.logoName = iconUrl.section(QLatin1Char('/'), -1)
                                        .section(QLatin1Char('.'), 0, 0);
                }
                else
                {
                    pack.logoUrl = "null";
                    pack.logoName = "null";
                }

                pack.broken = false;
                newList.append(pack);
                break;
            }
        }
    }
    catch (const JSONValidationError& err)
    {
        qCritical() << "Couldn't parse technic search results:" << err.cause();
        return;
    }
    searchState = Finished;

    // When you have a Qt build with assertions turned on, proceeding here will
    // abort the application
    if (newList.size() == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), modpacks.size(),
                    modpacks.size() + newList.size() - 1);
    modpacks.append(newList);
    endInsertRows();
}

QString Technic::ListModel::logoPath(const QString& logo, const QString& url)
{
    return m_logos->path(logo, url);
}

void Technic::ListModel::searchRequestFailed()
{
    jobPtr.reset();

    if (searchState == ResetRequested)
    {
        beginResetModel();
        modpacks.clear();
        endResetModel();

        performSearch();
    }
    else
    {
        searchState = Finished;
    }
}

void Technic::ListModel::onLogoLoaded(const QString& logo)
{
    for (int i = 0; i < modpacks.size(); i++)
    {
        if (modpacks[i].logoName == logo)
        {
            emit dataChanged(createIndex(i, 0), createIndex(i, 0),
                             {Qt::DecorationRole});
        }
    }
}
