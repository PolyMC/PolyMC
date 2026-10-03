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

#include <QAbstractListModel>

#include "modplatform/modrinth/ModrinthPackManifest.h"
#include "ui/pages/modplatform/modrinth/ModrinthPage.h"

class ModPage;
class Version;

namespace Modrinth {

using LogoMap = QMap<QString, QIcon>;
using LogoCallback = std::function<void (QString)>;

class ModpackListModel : public QAbstractListModel {
    Q_OBJECT

   public:
    ModpackListModel(ModrinthPage* parent);
    ~ModpackListModel() override = default;

    inline int rowCount(const QModelIndex& parent) const override { return modpacks.size(); };
    inline int columnCount(const QModelIndex& parent) const override { return 1; };
    inline Qt::ItemFlags flags(const QModelIndex& index) const override { return QAbstractListModel::flags(index); };

    QString debugName() const;

    /* Retrieve information from the model at a given index with the given role */
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    inline void setActiveJob(NetJob::Ptr ptr) { jobPtr = ptr; }

    /* Ask the API for more information */
    void fetchMore(const QModelIndex& parent) override;
    void refresh();
    void searchWithTerm(const QString& term, const int sort);

    void getLogo(const QString& logo, const QString& logoUrl, LogoCallback callback);

    inline bool canFetchMore(const QModelIndex& parent) const override { return searchState == CanPossiblyFetchMore; };

   public slots:
    void searchRequestFinished(QJsonDocument& doc_all);
    void searchRequestFailed(QString reason);

   protected slots:

    void logoFailed(QString logo);
    void logoLoaded(QString logo, QIcon out);

    void performPaginatedSearch();

   protected:
    void requestLogo(QString file, QString url);

    inline std::list<Version> getMineVersions() const;

   protected:
    ModrinthPage* m_parent;

    QList<Modrinth::Modpack> modpacks;

    LogoMap m_logoMap;
    QMap<QString, LogoCallback> waitingCallbacks;
    QStringList m_failedLogos;
    QStringList m_loadingLogos;

    QString currentSearchTerm;
    QString currentSort;
    int nextSearchOffset = 0;
    enum SearchState { None, CanPossiblyFetchMore, ResetRequested, Finished } searchState = None;

    NetJob::Ptr jobPtr;

    QByteArray m_all_response;
    QByteArray m_specific_response;

    int m_modpacks_per_page = 20;
};
}  // namespace ModPlatform
