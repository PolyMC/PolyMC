// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 Slendi <slendi@socopon.com>
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
 */

#pragma once

#include <QWidget>

#include "BaseInstance.h"
#include "ui/pages/BasePage.h"
#include <Application.h>

namespace Ui
{
class StoragePage;
}

class StoragePage : public QWidget, public BasePage
{
    Q_OBJECT

public:
    explicit StoragePage(BaseInstance *inst, QWidget *parent = 0);
    virtual ~StoragePage();
    virtual QString displayName() const override
    {
        return tr("Storage");
    }
    virtual QIcon icon() const override
    {
        auto icon = APPLICATION->getThemedIcon("notes");
        if(icon.isNull())
            icon = APPLICATION->getThemedIcon("news");
        return icon;
    }
    virtual QString id() const override
    {
        return "storage";
    }
    virtual bool apply() override;
    virtual QString helpPage() const override
    {
        return "Storage";
    }
    void retranslate() override;

    void HandleClearScreenshotsButton();
    void HandleClearLogsButton();
    void HandleClearAllButton();

    void update_calculations();

private:
    Ui::StoragePage *ui;
    BaseInstance *m_inst;

    unsigned m_size_resource_packs, m_size_mods, m_size_saves, m_size_screenshots, m_size_logs;
};
