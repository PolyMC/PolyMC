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

#include <QIcon>

Technic::ListModel::ListModel(QObject *parent) : QAbstractListModel(parent)
{
}

Technic::ListModel::~ListModel() = default;

QVariant Technic::ListModel::data(const QModelIndex& index, int role) const
{
    int pos = index.row();
    if(pos >= modpacks.size() || pos < 0 || !index.isValid())
    {
        return QString("INVALID INDEX %1").arg(pos);
    }

    Modpack pack = modpacks.at(pos);
    if(role == Qt::DisplayRole)
    {
        return pack.name;
    }
    else if(role == Qt::DecorationRole)
    {
        if(m_logoMap.contains(pack.logoName))
        {
            return (m_logoMap.value(pack.logoName));
        }
        QIcon icon = APPLICATION->getThemedIcon("screenshot-placeholder");
        ((ListModel *)this)->requestLogo(pack.logoName, pack.logoUrl);
        return icon;
    }
    else if(role == Qt::UserRole)
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
    if(currentSearchTerm == term && currentSearchTerm.isNull() == term.isNull()) {
        return;
    }
    currentSearchTerm = term;
    if(jobPtr) {
        jobPtr->abort();
        searchState = ResetRequested;
        return;
    }
    else {
        beginResetModel();
        modpacks.clear();
        endResetModel();
        searchState = None;
    }
    performSearch();
}

void Technic::ListModel::performSearch()
{
    NetJob *netJob = new NetJob("Technic::Search", APPLICATION->network());
    QString searchUrl = "";
    if (currentSearchTerm.isEmpty()) {
        searchUrl = QString("%1trending?build=%2")
                .arg(BuildConfig.TECHNIC_API_BASE_URL, BuildConfig.TECHNIC_API_BUILD);
        searchMode = List;
    }
    else if (currentSearchTerm.startsWith("http://api.technicpack.net/modpack/")) {
        searchUrl = QString("https://%1?build=%2")
                .arg(currentSearchTerm.mid(7), BuildConfig.TECHNIC_API_BUILD);
        searchMode = Single;
    }
    else if (currentSearchTerm.startsWith("https://api.technicpack.net/modpack/")) {
        searchUrl = QString("%1?build=%2").arg(currentSearchTerm, BuildConfig.TECHNIC_API_BUILD);
        searchMode = Single;
    }
    else {
        searchUrl = QString(
            "%1search?build=%2&q=%3"
        ).arg(BuildConfig.TECHNIC_API_BASE_URL, BuildConfig.TECHNIC_API_BUILD, currentSearchTerm);
        searchMode = List;
    }
    netJob->addNetAction(Net::Download::makeByteArray(QUrl(searchUrl), &response));
    jobPtr = netJob;
    jobPtr->start();
    QObject::connect(netJob, &NetJob::succeeded, this, &ListModel::searchRequestFinished);
    QObject::connect(netJob, &NetJob::failed, this, &ListModel::searchRequestFailed);
}

void Technic::ListModel::searchRequestFinished()
{
    jobPtr.reset();

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response.constData(), response.constData() + response.size());
    }
    catch (nlohmann::json::parse_error &e) {
        qWarning() << "Error while parsing JSON response from Technic at " << e.byte << " reason: " << e.what();
        qWarning() << response;
        return;
    }

    QList<Modpack> newList;
    try {
        switch (searchMode) {
            case List: {
                const auto& objs = root["modpacks"];
                for (const auto& technicPackObject: objs) {
                    Modpack pack;
                    pack.name = technicPackObject["name"].get<std::string>().c_str();
                    pack.slug = technicPackObject["slug"].get<std::string>().c_str();
                    if (pack.slug == "vanilla")
                        continue;

                    const nlohmann::json& temp = technicPackObject["iconUrl"];
                    if (!temp.is_null()) {
                        QString rawURL = temp.get<std::string>().c_str();
                        pack.logoUrl = rawURL;
                        pack.logoName = rawURL.section(QLatin1Char('/'), -1).section(QLatin1Char('.'), 0, 0);
                    }
                    else {
                        pack.logoUrl = "null";
                        pack.logoName = "null";
                    }

                    pack.broken = false;
                    newList.append(pack);
                }
                break;
            }
            case Single: {
                if (root.contains("error")) {
                    // Invalid API url
                    break;
                }

                Modpack pack;
                pack.name = root["displayName"].get<std::string>().c_str();
                pack.slug = root["name"].get<std::string>().c_str();

                if (root.contains("icon")) {
                    QString iconUrl = root["icon"]["url"].get<std::string>().c_str();

                    pack.logoUrl = iconUrl;
                    pack.logoName = iconUrl.section(QLatin1Char('/'), -1).section(QLatin1Char('.'), 0, 0);
                }
                else {
                    pack.logoUrl = "null";
                    pack.logoName = "null";
                }

                pack.broken = false;
                newList.append(pack);
                break;
            }
        }
    }
    catch (const nlohmann::json::exception &err)
    {
        qCritical() << "Couldn't parse technic search results:" << err.what();
        return;
    }
    searchState = Finished;

    // When you have a Qt build with assertions turned on, proceeding here will abort the application
    if (newList.empty())
        return;

    beginInsertRows(QModelIndex(), modpacks.size(), modpacks.size() + newList.size() - 1);
    modpacks.append(newList);
    endInsertRows();
}

void Technic::ListModel::getLogo(const QString& logo, const QString& logoUrl, Technic::LogoCallback callback)
{
    if(m_logoMap.contains(logo))
    {
        callback(APPLICATION->metacache()->resolveEntry("TechnicPacks", QString("logos/%1").arg(logo))->getFullPath());
    }
    else
    {
        requestLogo(logo, logoUrl);
    }
}

void Technic::ListModel::searchRequestFailed()
{
    jobPtr.reset();

    if(searchState == ResetRequested)
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


void Technic::ListModel::logoLoaded(const QString& logo, const QString& out)
{
    m_loadingLogos.removeAll(logo);
    m_logoMap.insert(logo, QIcon(out));
    for(int i = 0; i < modpacks.size(); i++)
    {
        if(modpacks[i].logoName == logo)
        {
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), {Qt::DecorationRole});
        }
    }
}

void Technic::ListModel::logoFailed(const QString& logo)
{
    m_failedLogos.append(logo);
    m_loadingLogos.removeAll(logo);
}

void Technic::ListModel::requestLogo(const QString& logo, const QString& url)
{
    if(m_loadingLogos.contains(logo) || m_failedLogos.contains(logo) || logo == "null")
    {
        return;
    }

    MetaEntryPtr entry = APPLICATION->metacache()->resolveEntry("TechnicPacks", QString("logos/%1").arg(logo));
    NetJob *job = new NetJob(QString("Technic Icon Download %1").arg(logo), APPLICATION->network());
    job->addNetAction(Net::Download::makeCached(QUrl(url), entry));

    auto fullPath = entry->getFullPath();

    QObject::connect(job, &NetJob::succeeded, this, [this, logo, fullPath]
    {
        logoLoaded(logo, fullPath);
    });

    QObject::connect(job, &NetJob::failed, this, [this, logo]
    {
        logoFailed(logo);
    });

    job->start();

    m_loadingLogos.append(logo);
}
