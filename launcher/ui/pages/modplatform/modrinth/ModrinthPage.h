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
 *      Copyright 2021-2022 kb1000
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

#include "Application.h"
#include "ui/dialogs/NewInstanceDialog.h"
#include "ui/pages/BasePage.h"

#include "modplatform/modrinth/ModrinthAPI.h"
#include "modplatform/modrinth/ModrinthPackManifest.h"
#include "modplatform/modrinth/ModrinthSource.h"
#include "net/NetJob.h"

#include <QByteArray>
#include <QIcon>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QString>
#include <QWidget>

namespace Ui
{
class ModrinthPage;
}

namespace Modrinth
{
class ModpackListModel;
}

class ModrinthPage : public QWidget, public BasePage
{
    Q_OBJECT

public:
    explicit ModrinthPage(NewInstanceDialog* dialog,
                          Modrinth::Source source = {},
                          QWidget* parent = nullptr);
    ~ModrinthPage() override;

    QString displayName() const override
    {
        if (m_api.source().isOfficial())
        {
            return tr("Modrinth");
        }
        return m_api.source().displayName();
    }
    QIcon icon() const override
    {
        return Modrinth::sourceIcon(m_api.source());
    }
    QString id() const override
    {
        return m_api.source().key();
    }
    QString helpPage() const override
    {
        return "Modrinth-platform";
    }

    inline QString debugName() const
    {
        return m_api.source().displayName();
    }
    inline QString metaEntryBase() const
    {
        return "ModrinthModpacks";
    };

    const ModrinthAPI& api() const
    {
        return m_api;
    }

    Modrinth::Modpack& getCurrent()
    {
        return current;
    }
    void suggestCurrent();

    void updateUI();

    void retranslate() override;
    void openedImpl() override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onSelectionChanged(QModelIndex first, QModelIndex second);
    void onVersionSelectionChanged(QString newData);
    void triggerSearch();

private:
    void onInfoLoaded();
    void onVersionsLoaded();

    Ui::ModrinthPage* ui;
    NewInstanceDialog* dialog;
    ModrinthAPI m_api;
    Modrinth::ModpackListModel* m_model;

    Modrinth::Modpack current;
    QString selectedVersion;

    NetJob::Ptr m_info_job;
    QByteArray m_info_response;
    QPersistentModelIndex m_info_index;

    NetJob::Ptr m_versions_job;
    QByteArray m_versions_response;
    QPersistentModelIndex m_versions_index;
};
