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
#include <QMessageBox>
#include <QStorageInfo>
#include <QTabBar>
#include "ui_StoragePage.h"

qint64 getDirectorySize(QString path)
{
    QDirIterator it(path, QDirIterator::Subdirectories);
    qint64 fileSizeTotal = 0;
    while (it.hasNext()) {
        it.next();
        fileSizeTotal += it.fileInfo().size();
    }
    return fileSizeTotal;
}

void clearDirectoryInner(QString path)
{
    QDir(path).removeRecursively();
}

StoragePage::StoragePage(BaseInstance* inst, QWidget* parent) : QWidget(parent), ui(new Ui::StoragePage), m_inst(inst)
{
    ui->setupUi(this);

    m_confirmation_box = new QMessageBox(this);
    m_confirmation_box->setWindowTitle("Confirmation");
    m_confirmation_box->setIcon(QMessageBox::Warning);
    m_confirmation_box->setText("Are you sure you want to proceed?");
    m_confirmation_box->setStandardButtons(QMessageBox::Yes);
    m_confirmation_box->addButton(QMessageBox::No);
    m_confirmation_box->setDefaultButton(QMessageBox::No);

    m_series = new QPieSeries(this);
    m_series->setLabelsVisible();
    m_series->setLabelsPosition(QPieSlice::LabelInsideHorizontal);
    m_chart_view = new QChartView(this);
    m_chart = new QChart();
    m_chart->setParent(this);
    m_chart->addSeries(m_series);
    m_chart->setBackgroundVisible(false);
    m_chart->setMargins(QMargins(0, 0, 0, 0));
    m_chart->legend()->setAlignment(Qt::AlignLeft);
    m_chart->legend()->setLabelColor(QApplication::palette().text().color());
    m_chart_view->setRenderHint(QPainter::Antialiasing);
    m_chart_view->installEventFilter(this);

    ui->verticalLayout->addWidget(m_chart_view);
    ui->retranslateUi(this);

    updateCalculations();

    connect(ui->button_goto_resouce_packs, &QPushButton::clicked, this, [&] { m_container->selectPage("resourcepacks"); });
    connect(ui->button_goto_mods, &QPushButton::clicked, this, [&] { m_container->selectPage("mods"); });
    connect(ui->button_goto_worlds, &QPushButton::clicked, this, [&] { m_container->selectPage("worlds"); });
    connect(ui->button_goto_screenshots, &QPushButton::clicked, this, [&] { m_container->selectPage("screenshots"); });
    connect(ui->button_goto_other_logs, &QPushButton::clicked, this, [&] { m_container->selectPage("logs"); });

    connect(ui->button_clear_screenshots, &QPushButton::clicked, this, &StoragePage::handleClearScreenshotsButton);
    connect(ui->button_clear_logs, &QPushButton::clicked, this, &StoragePage::handleClearLogsButton);
    connect(ui->button_clear_all, &QPushButton::clicked, this, &StoragePage::handleClearAllButton);
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

void StoragePage::handleClearScreenshotsButton()
{
    if (m_confirmation_box->exec() != QMessageBox::Yes)
        return;

    auto path = m_inst->gameRoot() + "/screenshots";
    QDir(path).removeRecursively();
    updateCalculations();
}

void StoragePage::handleClearLogsButton()
{
    if (m_confirmation_box->exec() != QMessageBox::Yes)
        return;

    auto path = m_inst->gameRoot() + "/logs";
    QDir(path).removeRecursively();
    updateCalculations();
}

void StoragePage::handleClearAllButton()
{
    if (m_confirmation_box->exec() != QMessageBox::Yes)
        return;

    handleClearScreenshotsButton();
    handleClearLogsButton();
}

void StoragePage::updateCalculations()
{
    auto size_resource_packs =
        getDirectorySize((m_inst->gameRoot() + "/texturepacks")) + getDirectorySize((m_inst->gameRoot() + "/resourcepacks"));
    auto size_mods = getDirectorySize(m_inst->modsRoot());
    auto size_saves = getDirectorySize((m_inst->gameRoot() + "/saves"));
    auto size_screenshots = getDirectorySize((m_inst->gameRoot() + "/screenshots"));
    auto size_logs = getDirectorySize((m_inst->gameRoot() + "/logs"));

    auto storage_info = QStorageInfo(QDir(m_inst->gameRoot()));
    auto size_remaining = storage_info.bytesAvailable();
    auto size_used = storage_info.bytesTotal() - size_remaining;

    auto locale = this->locale();
    ui->label_resource_packs->setText(locale.formattedDataSize(size_resource_packs));
    ui->label_mods->setText(locale.formattedDataSize(size_mods));
    ui->label_saves->setText(locale.formattedDataSize(size_saves));
    ui->label_screenshots->setText(locale.formattedDataSize(size_screenshots));
    ui->label_logs->setText(locale.formattedDataSize(size_logs));
    ui->label_combined->setText(locale.formattedDataSize(size_resource_packs + size_mods + size_saves + size_screenshots + size_logs));

    ui->label_used->setText(locale.formattedDataSize(size_used));
    ui->label_remaining->setText(locale.formattedDataSize(size_remaining));

    m_series->clear();
    m_series->append("Resource packs", size_resource_packs);
    m_series->append("Mods", size_mods);
    m_series->append("Saves", size_saves);
    m_series->append("Screenshots", size_screenshots);
    m_series->append("Logs", size_logs);

    m_chart_view->setChart(m_chart);
    for (auto slice : m_series->slices())
        slice->setLabel(slice->label() + " " + QString("%1%").arg(100 * slice->percentage(), 0, 'f', 1));
}

bool StoragePage::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::PaletteChange)
    {
        m_chart->legend()->setLabelColor(QApplication::palette().text().color());
        return true;
    }
    return false;
}
