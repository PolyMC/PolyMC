// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
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

#include "FlamePage.h"
#include "ui_FlamePage.h"

#include <QKeyEvent>
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QItemSelectionModel>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QPushButton>
#include <QStringList>
#include <QVariant>

#include <utility>

#include "Application.h"
#include "FlameModel.h"
#include "InstanceImportTask.h"
#include "Json.h"
#include "net/Download.h"
#include "ui/dialogs/NewInstanceDialog.h"

namespace
{
QString authorToStr(Flame::ModpackAuthor& author)
{
    if (author.url.isEmpty())
    {
        return author.name;
    }
    return QString("<a href=\"%1\">%2</a>").arg(author.url, author.name);
}
}  // namespace

FlamePage::FlamePage(NewInstanceDialog* dialog, QWidget* parent)
    : QWidget(parent), ui(new Ui::FlamePage), dialog(dialog)
{
    ui->setupUi(this);
    connect(ui->searchButton, &QPushButton::clicked, this,
            &FlamePage::triggerSearch);
    ui->searchEdit->installEventFilter(this);
    listModel = new Flame::ListModel(this);
    ui->packView->setModel(listModel);

    ui->versionSelectionBox->view()->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded);
    ui->versionSelectionBox->view()->parentWidget()->setMaximumHeight(300);

    // index is used to set the sorting with the curseforge api
    ui->sortByBox->addItem(tr("Sort by Featured"));
    ui->sortByBox->addItem(tr("Sort by Popularity"));
    ui->sortByBox->addItem(tr("Sort by Last Updated"));
    ui->sortByBox->addItem(tr("Sort by Name"));
    ui->sortByBox->addItem(tr("Sort by Author"));
    ui->sortByBox->addItem(tr("Sort by Total Downloads"));

    connect(ui->sortByBox, &QComboBox::currentIndexChanged, this,
            &FlamePage::triggerSearch);
    connect(ui->packView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &FlamePage::onSelectionChanged);
    connect(ui->versionSelectionBox, &QComboBox::currentTextChanged, this,
            &FlamePage::onVersionSelectionChanged);
}

FlamePage::~FlamePage()
{
    if (m_versions_job && m_versions_job->isRunning())
    {
        m_versions_job->disconnect(this);
        m_versions_job->abort();
    }
    delete ui;
}

bool FlamePage::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->searchEdit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return)
        {
            triggerSearch();
            keyEvent->accept();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

bool FlamePage::shouldDisplay() const
{
    return true;
}

void FlamePage::retranslate()
{
    ui->retranslateUi(this);
}

void FlamePage::openedImpl()
{
    suggestCurrent();
    triggerSearch();
}

void FlamePage::triggerSearch()
{
    listModel->searchWithTerm(ui->searchEdit->text(),
                              ui->sortByBox->currentIndex());
}

void FlamePage::onSelectionChanged(QModelIndex curr, QModelIndex /*prev*/)
{
    ui->versionSelectionBox->clear();

    if (!curr.isValid())
    {
        if (isOpened)
        {
            dialog->setSuggestedPack();
        }
        return;
    }

    if (m_versions_job && m_versions_job->isRunning())
    {
        m_versions_job->disconnect(this);
        m_versions_job->abort();
    }

    current = listModel->data(curr, Qt::UserRole).value<Flame::IndexedPack>();

    if (current.versionsLoaded == false)
    {
        qDebug() << "Loading flame modpack versions";
        m_versions_index = curr;
        m_versions_job =
            new NetJob(QString("Flame::PackVersions(%1)").arg(current.name),
                       APPLICATION->network());
        m_versions_job->addNetAction(Net::Download::makeByteArray(
            QString("https://api.curseforge.com/v1/mods/%1/files")
                .arg(current.addonId),
            &m_versions_response));

        connect(m_versions_job.get(), &NetJob::succeeded, this,
                &FlamePage::onVersionsLoaded);
        m_versions_job->start();
    }
    else
    {
        for (const auto& version : std::as_const(current.versions))
        {
            ui->versionSelectionBox->addItem(version.version,
                                             QVariant(version.downloadUrl));
        }

        suggestCurrent();
    }

    // TODO: Check whether it's a connection issue or the project disabled
    // 3rd-party distribution.
    if (current.versionsLoaded && ui->versionSelectionBox->count() < 1)
    {
        ui->versionSelectionBox->addItem(tr("No version is available!"), -1);
    }

    updateUi();
}

void FlamePage::onVersionsLoaded()
{
    QJsonParseError parse_error;
    QJsonDocument doc =
        QJsonDocument::fromJson(m_versions_response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from "
                      "CurseForge at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << m_versions_response;
        return;
    }
    auto arr = Json::ensureArray(doc.object(), "data");
    try
    {
        Flame::loadIndexedPackVersions(current, arr);
    }
    catch (const JSONValidationError& e)
    {
        qDebug() << m_versions_response;
        qWarning() << "Error while reading flame modpack version: "
                   << e.cause();
    }

    for (const auto& version : std::as_const(current.versions))
    {
        ui->versionSelectionBox->addItem(version.version,
                                         QVariant(version.downloadUrl));
    }

    // Only cache it if the row still holds this pack, a new search
    // may have put another one there meanwhile
    if (listModel->data(m_versions_index, Qt::UserRole)
            .value<Flame::IndexedPack>()
            .addonId == current.addonId)
    {
        QVariant current_updated;
        current_updated.setValue(current);

        if (!listModel->setData(m_versions_index, current_updated,
                                Qt::UserRole))
        {
            qWarning() << "Failed to cache versions for the current pack!";
        }
    }

    // TODO: Check whether it's a connection issue or the project
    // disabled 3rd-party distribution.
    if (current.versionsLoaded && ui->versionSelectionBox->count() < 1)
    {
        ui->versionSelectionBox->addItem(tr("No version is available!"), -1);
    }
    suggestCurrent();
}

void FlamePage::suggestCurrent()
{
    if (!isOpened)
    {
        return;
    }

    if (selectedVersion.isEmpty() || selectedVersion == "-1")
    {
        dialog->setSuggestedPack();
        return;
    }

    dialog->setSuggestedPack(current.name,
                             new InstanceImportTask(selectedVersion, this));
    QString editedLogoName;
    editedLogoName = "curseforge_" + current.logoName.section(".", 0, 0);
    auto path = listModel->logoPath(current.logoName, current.logoUrl);
    if (!path.isEmpty())
    {
        dialog->setSuggestedIconFromFile(path, editedLogoName);
    }
}

void FlamePage::onVersionSelectionChanged(QString newData)
{
    if (newData.isNull() || newData.isEmpty())
    {
        selectedVersion = "";
        return;
    }
    selectedVersion = ui->versionSelectionBox->currentData().toString();
    suggestCurrent();
}

void FlamePage::updateUi()
{
    QString text = "";
    QString name = current.name;

    if (current.extra.websiteUrl.isEmpty())
    {
        text = name;
    }
    else
    {
        text = "<a href=\"" + current.extra.websiteUrl + "\">" + name + "</a>";
    }
    if (!current.authors.empty())
    {
        QStringList authorStrs;
        for (auto& author : current.authors)
        {
            authorStrs.push_back(authorToStr(author));
        }
        text += "<br>" + tr(" by ") + authorStrs.join(", ");
    }

    if (current.extraInfoLoaded)
    {
        if (!current.extra.issuesUrl.isEmpty() ||
            !current.extra.sourceUrl.isEmpty() ||
            !current.extra.wikiUrl.isEmpty())
        {
            text += "<br><br>" + tr("External links:") + "<br>";
        }

        if (!current.extra.issuesUrl.isEmpty())
        {
            text +=
                "- " +
                tr("Issues: <a href=%1>%1</a>").arg(current.extra.issuesUrl) +
                "<br>";
        }
        if (!current.extra.wikiUrl.isEmpty())
        {
            text += "- " +
                    tr("Wiki: <a href=%1>%1</a>").arg(current.extra.wikiUrl) +
                    "<br>";
        }
        if (!current.extra.sourceUrl.isEmpty())
        {
            text += "- " +
                    tr("Source code: <a href=%1>%1</a>")
                        .arg(current.extra.sourceUrl) +
                    "<br>";
        }
    }

    text += "<hr>";

    ui->packDescription->setHtml(text + current.description);
}
