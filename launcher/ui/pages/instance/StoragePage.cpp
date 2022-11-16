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
 */

#include "StoragePage.h"
#include <QTabBar>
#include "ui_StoragePage.h"

#include <filesystem>
#include <string>

unsigned get_directory_size(std::string path)
{
    if (!std::filesystem::exists(path))
        return 0;
    unsigned file_size_total = 0;
    for (auto const& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory())
            file_size_total += get_directory_size(entry.path().string());
        else
            file_size_total += entry.file_size();
    }
    return file_size_total;
}

void clear_directory_inner(std::string path)
{
    if (!std::filesystem::exists(path))
        return;

    for (auto const& entry : std::filesystem::directory_iterator(path))
        std::filesystem::remove_all(entry);
}

StoragePage::StoragePage(BaseInstance* inst, QWidget* parent) : QWidget(parent), ui(new Ui::StoragePage), m_inst(inst)
{
    ui->setupUi(this);

    update_calculations();
}

StoragePage::~StoragePage()
{
    delete ui;
}

bool StoragePage::apply()
{
    return true;
}

void StoragePage::retranslate()
{
    ui->retranslateUi(this);
}

void StoragePage::HandleClearScreenshotsButton()
{
    auto path = (m_inst->gameRoot() + "/screenshots").toStdString();
    clear_directory_inner(path);
    update_calculations();
}

void StoragePage::HandleClearLogsButton()
{
    auto path = (m_inst->gameRoot() + "/logs").toStdString();
    clear_directory_inner(path);
    update_calculations();
}

void StoragePage::HandleClearAllButton()
{
    HandleClearScreenshotsButton();
    HandleClearLogsButton();
}

void StoragePage::update_calculations()
{
    m_size_resource_packs = get_directory_size((m_inst->gameRoot() + "/texturepacks").toStdString()) +
                            get_directory_size((m_inst->gameRoot() + "/resourcepacks").toStdString());
    m_size_mods = get_directory_size(m_inst->modsRoot().toStdString());
    m_size_saves = get_directory_size((m_inst->gameRoot() + "/saves").toStdString());
    m_size_screenshots = get_directory_size((m_inst->gameRoot() + "/screenshots").toStdString());
    m_size_logs = get_directory_size((m_inst->gameRoot() + "/logs").toStdString());

    QLocale locale = this->locale();
    ui->label_resource_packs->setText(locale.formattedDataSize(m_size_resource_packs));
    ui->label_mods->setText(locale.formattedDataSize(m_size_mods));
    ui->label_saves->setText(locale.formattedDataSize(m_size_saves));
    ui->label_screenshots->setText(locale.formattedDataSize(m_size_screenshots));
    ui->label_logs->setText(locale.formattedDataSize(m_size_logs));
    ui->label_combined->setText(
        locale.formattedDataSize(m_size_resource_packs + m_size_mods + m_size_saves + m_size_screenshots + m_size_logs));

    connect(ui->button_clear_screenshots, &QPushButton::clicked, this, &StoragePage::HandleClearScreenshotsButton);
    connect(ui->button_clear_logs, &QPushButton::clicked, this, &StoragePage::HandleClearLogsButton);
    connect(ui->button_clear_all, &QPushButton::clicked, this, &StoragePage::HandleClearAllButton);
}
