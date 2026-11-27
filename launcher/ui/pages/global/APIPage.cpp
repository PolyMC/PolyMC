// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
 *  Copyright (c) 2022 Lenny McLennington <lenny@sneed.church>
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

#include "APIPage.h"
#include "ui_APIPage.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUrl>

#include "settings/SettingsObject.h"
#include "Application.h"
#include "minecraft/AssetsUtils.h"
#include "modplatform/modrinth/ModrinthSource.h"
#include "net/PasteUpload.h"
#include "BuildConfig.h"

#include "ui/GuiUtil.h"
#include "ui/dialogs/ModrinthSourceDialog.h"

namespace
{
enum ModrinthSourceColumn
{
    SourceNameColumn = 0,
    SourceApiColumn,
    SourceWebsiteColumn,
    SourceTokenColumn,
};

// Whether a source with the same API URL and website was there before
bool hasSameModrinthSite(const QList<Modrinth::Source>& sources,
                         const Modrinth::Source& source)
{
    for (const auto& other : sources)
    {
        if (other.api_url == source.api_url &&
            other.website_url == source.website_url)
        {
            return true;
        }
    }
    return false;
}
}  // namespace

APIPage::APIPage(QWidget* parent) : QWidget(parent), ui(new Ui::APIPage)
{
    // This is here so you can reorder the entries in the combobox without
    // messing stuff up
    int comboBoxEntries[] = {
        PasteUpload::PasteType::Mclogs, PasteUpload::PasteType::NullPointer,
        PasteUpload::PasteType::PasteGG, PasteUpload::PasteType::Hastebin};

    static QRegularExpression validUrlRegExp("https?://.+");
    static QRegularExpression validMSAClientID(
        QRegularExpression::anchoredPattern(
            "[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{"
            "12}"));
    static QRegularExpression validFlameKey(
        QRegularExpression::anchoredPattern("\\$2[ayb]\\$.{56}"));

    ui->setupUi(this);

    for (auto pasteType : comboBoxEntries)
    {
        ui->pasteTypeComboBox->addItem(
            PasteUpload::PasteTypes.at(pasteType).name, pasteType);
    }

    void (QComboBox::*currentIndexChangedSignal)(int)(
        &QComboBox::currentIndexChanged);
    connect(ui->pasteTypeComboBox, currentIndexChangedSignal, this,
            &APIPage::updateBaseURLPlaceholder);
    // This function needs to be called even when the ComboBox's index is still
    // in its default state.
    updateBaseURLPlaceholder(ui->pasteTypeComboBox->currentIndex());
    ui->baseURLEntry->setValidator(
        new TrimmedRegExValidator(validUrlRegExp, ui->baseURLEntry));
    ui->msaClientID->setValidator(
        new TrimmedRegExValidator(validMSAClientID, ui->msaClientID));
    ui->flameKey->setValidator(
        new TrimmedRegExValidator(validFlameKey, ui->flameKey));

    ui->metaURL->setPlaceholderText(BuildConfig.META_URL);
    ui->userAgentLineEdit->setPlaceholderText(BuildConfig.USER_AGENT);

    if (BuildConfig.FLAME_API_KEY_API_URL.isEmpty())
    {
        ui->fetchKeyButton->hide();
    }

    loadSettings();

    resetBaseURLNote();
    connect(ui->pasteTypeComboBox, currentIndexChangedSignal, this,
            &APIPage::updateBaseURLNote);
    connect(ui->baseURLEntry, &QLineEdit::textEdited, this,
            &APIPage::resetBaseURLNote);

    connect(ui->fetchKeyButton, &QPushButton::clicked, this,
            &APIPage::fetchKeyButtonPressed);

    connect(ui->addModrinthSourceButton, &QPushButton::clicked, this,
            &APIPage::addModrinthSource);
    connect(ui->editModrinthSourceButton, &QPushButton::clicked, this,
            &APIPage::editModrinthSource);
    connect(ui->removeModrinthSourceButton, &QPushButton::clicked, this,
            &APIPage::removeModrinthSource);
    connect(ui->modrinthSourcesTree, &QTreeWidget::itemDoubleClicked, this,
            &APIPage::editModrinthSource);
}

APIPage::~APIPage()
{
    delete ui;
}

void APIPage::resetBaseURLNote()
{
    ui->baseURLNote->hide();
    baseURLPasteType = ui->pasteTypeComboBox->currentIndex();
}

void APIPage::updateBaseURLNote(int index)
{
    if (baseURLPasteType == index)
    {
        ui->baseURLNote->hide();
    }
    else if (!ui->baseURLEntry->text().isEmpty())
    {
        ui->baseURLNote->show();
    }
}

void APIPage::updateBaseURLPlaceholder(int index)
{
    int pasteType = ui->pasteTypeComboBox->itemData(index).toInt();
    QString pasteDefaultURL = PasteUpload::PasteTypes.at(pasteType).defaultBase;
    ui->baseURLEntry->setPlaceholderText(pasteDefaultURL);
}

void APIPage::loadSettings()
{
    auto s = APPLICATION->settings();

    int pasteType = s->get("PastebinType").toInt();
    QString pastebinURL = s->get("PastebinCustomAPIBase").toString();

    ui->baseURLEntry->setText(pastebinURL);
    int pasteTypeIndex = ui->pasteTypeComboBox->findData(pasteType);
    if (pasteTypeIndex == -1)
    {
        pasteTypeIndex =
            ui->pasteTypeComboBox->findData(PasteUpload::PasteType::Mclogs);
        ui->baseURLEntry->clear();
    }

    ui->pasteTypeComboBox->setCurrentIndex(pasteTypeIndex);

    QString msaClientID = s->get("MSAClientIDOverride").toString();
    ui->msaClientID->setText(msaClientID);
    QString metaURL = s->get("MetaURLOverride").toString();
    ui->metaURL->setText(metaURL);
    QString flameKey = s->get("FlameKeyOverride").toString();
    ui->flameKey->setText(flameKey);
    QString customUserAgent = s->get("UserAgentOverride").toString();
    ui->userAgentLineEdit->setText(customUserAgent);

    ui->modrinthToken->setText(s->get("ModrinthToken").toString());
    loadModrinthSources();
}

void APIPage::applySettings()
{
    auto s = APPLICATION->settings();

    s->set("PastebinType", ui->pasteTypeComboBox->currentData().toInt());
    s->set("PastebinCustomAPIBase", ui->baseURLEntry->text());

    QString msaClientID = ui->msaClientID->text();
    s->set("MSAClientIDOverride", msaClientID);
    QUrl metaURL = ui->metaURL->text();
    // Add required trailing slash
    if (!metaURL.isEmpty() && !metaURL.path().endsWith('/'))
    {
        QString path = metaURL.path();
        path.append('/');
        metaURL.setPath(path);
    }
    // Don't allow HTTP for remote metadata, since meta is basically RCE with
    // all the jar files. Local HTTP is useful for development and offline
    // metadata mirrors.
    if (!metaURL.isEmpty() && metaURL.scheme() == "http" &&
        !AssetsUtils::isLocalMetadataHost(metaURL.host()))
    {
        metaURL.setScheme("https");
    }

    s->set("MetaURLOverride", metaURL);
    QString flameKey = ui->flameKey->text();
    s->set("FlameKeyOverride", flameKey);
    s->set("UserAgentOverride", ui->userAgentLineEdit->text());

    s->set("ModrinthToken", ui->modrinthToken->text().trimmed());
    applyModrinthSources();

    APPLICATION->updateCapabilities();
}

void APIPage::loadModrinthSources()
{
    m_modrinth_sources = Modrinth::customSources();
    refreshModrinthSources();
}

void APIPage::refreshModrinthSources()
{
    ui->modrinthSourcesTree->clear();
    for (const auto& source : std::as_const(m_modrinth_sources))
    {
        auto* item = new QTreeWidgetItem(
            ui->modrinthSourcesTree,
            {source.displayName(), source.api_url, source.website_url,
             source.token.isEmpty() ? QString() : tr("Set")});
        item->setIcon(SourceNameColumn, Modrinth::sourceIcon(source));
    }
    for (int i = 0; i < ui->modrinthSourcesTree->columnCount(); i++)
    {
        ui->modrinthSourcesTree->resizeColumnToContents(i);
    }
}

QStringList APIPage::otherModrinthApiUrls(int row) const
{
    QStringList api_urls;
    for (int i = 0; i < m_modrinth_sources.size(); i++)
    {
        if (i != row)
        {
            api_urls.append(m_modrinth_sources.at(i).api_url);
        }
    }
    return api_urls;
}

void APIPage::applyModrinthSources()
{
    auto old_sources = Modrinth::customSources();
    Modrinth::setCustomSources(m_modrinth_sources);

    // New sites, or sites with another website, may have another icon
    for (const auto& source : std::as_const(m_modrinth_sources))
    {
        if (!hasSameModrinthSite(old_sources, source))
        {
            Modrinth::fetchSourceIcon(source, true);
        }
    }
}

void APIPage::addModrinthSource()
{
    ModrinthSourceDialog dialog(this, {}, otherModrinthApiUrls(-1));
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    m_modrinth_sources.append(dialog.source());
    refreshModrinthSources();
    ui->modrinthSourcesTree->setCurrentItem(
        ui->modrinthSourcesTree->topLevelItem(m_modrinth_sources.size() - 1));
}

void APIPage::editModrinthSource()
{
    auto row = ui->modrinthSourcesTree->indexOfTopLevelItem(
        ui->modrinthSourcesTree->currentItem());
    if (row < 0)
    {
        return;
    }

    ModrinthSourceDialog dialog(this, m_modrinth_sources.at(row),
                                otherModrinthApiUrls(row));
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    m_modrinth_sources[row] = dialog.source();
    refreshModrinthSources();
    ui->modrinthSourcesTree->setCurrentItem(
        ui->modrinthSourcesTree->topLevelItem(row));
}

void APIPage::removeModrinthSource()
{
    auto row = ui->modrinthSourcesTree->indexOfTopLevelItem(
        ui->modrinthSourcesTree->currentItem());
    if (row < 0)
    {
        return;
    }

    m_modrinth_sources.removeAt(row);
    refreshModrinthSources();
}

void APIPage::fetchKeyButtonPressed()
{
    QString apiKey = GuiUtil::fetchFlameKey(parentWidget());

    if (!apiKey.isEmpty())
    {
        ui->flameKey->setText(apiKey);
    }
}

bool APIPage::apply()
{
    applySettings();
    return true;
}

void APIPage::retranslate()
{
    ui->retranslateUi(this);
}

#include "moc_APIPage.cpp"
