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

#include "ModrinthPage.h"
#include "ui_ModrinthPage.h"

#include "ModrinthModel.h"

#include "InstanceImportTask.h"
#include "Json.h"
#include "modplatform/modrinth/ModrinthAPI.h"
#include "net/Download.h"

#include "ui/widgets/ProjectItem.h"

#include "Markdown.h"

#include <QComboBox>
#include <QDebug>
#include <QItemSelectionModel>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QKeyEvent>
#include <QPushButton>
#include <QStringList>
#include <QVariant>

#include <tuple>
#include <utility>

static ModrinthAPI modrinth_modpack_page_api;

namespace
{
void stopJob(NetJob* job, QObject* receiver)
{
    if (job && job->isRunning())
    {
        job->disconnect(receiver);
        job->abort();
    }
}

QString donateToStr(Modrinth::DonationData& donate)
{
    return QString("<a href=\"%1\">%2</a>").arg(donate.url, donate.platform);
}
}  // namespace

ModrinthPage::ModrinthPage(NewInstanceDialog* dialog, QWidget* parent)
    : QWidget(parent), ui(new Ui::ModrinthPage), dialog(dialog)
{
    ui->setupUi(this);

    connect(ui->searchButton, &QPushButton::clicked, this,
            &ModrinthPage::triggerSearch);
    ui->searchEdit->installEventFilter(this);
    m_model = new Modrinth::ModpackListModel(this);
    ui->packView->setModel(m_model);

    ui->versionSelectionBox->view()->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded);
    ui->versionSelectionBox->view()->parentWidget()->setMaximumHeight(300);

    ui->sortByBox->addItem(tr("Sort by Relevance"));
    ui->sortByBox->addItem(tr("Sort by Total Downloads"));
    ui->sortByBox->addItem(tr("Sort by Follows"));
    ui->sortByBox->addItem(tr("Sort by Newest"));
    ui->sortByBox->addItem(tr("Sort by Last Updated"));

    connect(ui->sortByBox, &QComboBox::currentIndexChanged, this,
            &ModrinthPage::triggerSearch);
    connect(ui->packView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &ModrinthPage::onSelectionChanged);
    connect(ui->versionSelectionBox, &QComboBox::currentTextChanged, this,
            &ModrinthPage::onVersionSelectionChanged);

    ui->packView->setItemDelegate(new ProjectItemDelegate(this));
}

ModrinthPage::~ModrinthPage()
{
    stopJob(m_info_job.get(), this);
    stopJob(m_versions_job.get(), this);
    delete ui;
}

void ModrinthPage::retranslate()
{
    ui->retranslateUi(this);
}

void ModrinthPage::openedImpl()
{
    BasePage::openedImpl();
    triggerSearch();
}

bool ModrinthPage::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->searchEdit && event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return)
        {
            this->triggerSearch();
            keyEvent->accept();
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

void ModrinthPage::onSelectionChanged(QModelIndex curr, QModelIndex /*prev*/)
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

    stopJob(m_info_job.get(), this);
    stopJob(m_versions_job.get(), this);

    current = m_model->data(curr, Qt::UserRole).value<Modrinth::Modpack>();

    if (!current.extraInfoLoaded)
    {
        qDebug() << "Loading modrinth modpack information";

        m_info_index = curr;
        m_info_job = new NetJob(
            QString("Modrinth::PackInformation(%1)").arg(current.name),
            APPLICATION->network());
        m_info_job->addNetAction(Net::Download::makeByteArray(
            QString("%1/project/%2")
                .arg(modrinth_modpack_page_api.apiRoot(), current.id),
            &m_info_response));

        connect(m_info_job.get(), &NetJob::succeeded, this,
                &ModrinthPage::onInfoLoaded);
        m_info_job->start();
    }
    else
    {
        updateUI();
    }

    if (!current.versionsLoaded)
    {
        qDebug() << "Loading modrinth modpack versions";

        m_versions_index = curr;
        m_versions_job =
            new NetJob(QString("Modrinth::PackVersions(%1)").arg(current.name),
                       APPLICATION->network());
        m_versions_job->addNetAction(Net::Download::makeByteArray(
            QString("%1/project/%2/version")
                .arg(modrinth_modpack_page_api.apiRoot(), current.id),
            &m_versions_response));

        connect(m_versions_job.get(), &NetJob::succeeded, this,
                &ModrinthPage::onVersionsLoaded);
        m_versions_job->start();
    }
    else
    {
        for (const auto& version : std::as_const(current.versions))
        {
            if (!version.name.contains(version.version))
            {
                ui->versionSelectionBox->addItem(
                    QString("%1 - %2").arg(version.name, version.version),
                    QVariant(version.id));
            }
            else
            {
                ui->versionSelectionBox->addItem(version.name,
                                                 QVariant(version.id));
            }
        }

        suggestCurrent();
    }
}

void ModrinthPage::onInfoLoaded()
{
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(m_info_response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from Modrinth at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << m_info_response;
        return;
    }

    try
    {
        // Inside the try block, this throws when the response isn't
        // an object
        auto obj = Json::requireObject(doc);
        Modrinth::loadIndexedInfo(current, obj);
    }
    catch (const JSONValidationError& e)
    {
        qDebug() << m_info_response;
        qWarning() << "Error while reading modrinth modpack version: "
                   << e.cause();
    }

    updateUI();

    // Only cache it if the row still holds this pack, a new search
    // may have put another one there meanwhile
    if (m_model->data(m_info_index, Qt::UserRole)
            .value<Modrinth::Modpack>()
            .id == current.id)
    {
        QVariant current_updated;
        current_updated.setValue(current);

        if (!m_model->setData(m_info_index, current_updated, Qt::UserRole))
        {
            qWarning() << "Failed to cache extra info for the "
                          "current pack!";
        }
    }

    suggestCurrent();
}

void ModrinthPage::onVersionsLoaded()
{
    QJsonParseError parse_error;
    QJsonDocument doc =
        QJsonDocument::fromJson(m_versions_response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from Modrinth at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << m_versions_response;
        return;
    }

    try
    {
        Modrinth::loadIndexedVersions(current, doc);
    }
    catch (const JSONValidationError& e)
    {
        qDebug() << m_versions_response;
        qWarning() << "Error while reading modrinth modpack version: "
                   << e.cause();
    }

    for (const auto& version : std::as_const(current.versions))
    {
        if (!version.name.contains(version.version))
        {
            ui->versionSelectionBox->addItem(
                QString("%1 — %2").arg(version.name, version.version),
                QVariant(version.id));
        }
        else
        {
            ui->versionSelectionBox->addItem(version.name,
                                             QVariant(version.id));
        }
    }

    // Only cache it if the row still holds this pack, a new search
    // may have put another one there meanwhile
    if (m_model->data(m_versions_index, Qt::UserRole)
            .value<Modrinth::Modpack>()
            .id == current.id)
    {
        QVariant current_updated;
        current_updated.setValue(current);

        if (!m_model->setData(m_versions_index, current_updated, Qt::UserRole))
        {
            qWarning() << "Failed to cache versions for the current pack!";
        }
    }

    suggestCurrent();
}

void ModrinthPage::updateUI()
{
    QString text = "";

    if (current.extra.projectUrl.isEmpty())
    {
        text = current.name;
    }
    else
    {
        text = "<a href=\"" + current.extra.projectUrl + "\">" + current.name +
               "</a>";
    }

    // TODO: Implement multiple authors with links
    text += "<br>" + tr(" by ") +
            QString("<a href=%1>%2</a>")
                .arg(std::get<1>(current.author).toString(),
                     std::get<0>(current.author));

    if (current.extraInfoLoaded)
    {
        if (!current.extra.donate.isEmpty())
        {
            text += "<br><br>" + tr("Donate information: ");
            QStringList donates;
            for (auto& donate : current.extra.donate)
            {
                donates.append(donateToStr(donate));
            }
            text += donates.join(", ");
        }

        if (!current.extra.issuesUrl.isEmpty() ||
            !current.extra.sourceUrl.isEmpty() ||
            !current.extra.wikiUrl.isEmpty() ||
            !current.extra.discordUrl.isEmpty())
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
        if (!current.extra.discordUrl.isEmpty())
        {
            text +=
                "- " +
                tr("Discord: <a href=%1>%1</a>").arg(current.extra.discordUrl) +
                "<br>";
        }
    }

    text += "<hr>";

    text += Markdown::toHtml(current.extra.body.toUtf8());

    ui->packDescription->setHtml(text + current.description);
}

void ModrinthPage::suggestCurrent()
{
    if (!isOpened)
    {
        return;
    }

    if (selectedVersion.isEmpty())
    {
        dialog->setSuggestedPack();
        return;
    }

    for (auto& ver : current.versions)
    {
        if (ver.id == selectedVersion)
        {
            dialog->setSuggestedPack(
                current.name, ver.version,
                new InstanceImportTask(ver.download_url, this));
            auto iconName = current.iconName;
            auto path = m_model->logoPath(iconName, current.iconUrl.toString());
            if (!path.isEmpty())
            {
                dialog->setSuggestedIconFromFile(path, iconName);
            }

            break;
        }
    }
}

void ModrinthPage::triggerSearch()
{
    m_model->searchWithTerm(ui->searchEdit->text(),
                            ui->sortByBox->currentIndex());
}

void ModrinthPage::onVersionSelectionChanged(QString newData)
{
    if (newData.isNull() || newData.isEmpty())
    {
        selectedVersion = "";
        return;
    }
    selectedVersion = ui->versionSelectionBox->currentData().toString();
    suggestCurrent();
}

#include "moc_ModrinthPage.cpp"
