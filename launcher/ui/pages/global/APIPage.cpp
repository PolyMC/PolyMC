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
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUrl>

#include "settings/SettingsObject.h"
#include "Application.h"
#include "minecraft/AssetsUtils.h"
#include "modplatform/modrinth/ModrinthSource.h"
#include "net/Download.h"
#include "net/PasteUpload.h"
#include "BuildConfig.h"

#include "ui/GuiUtil.h"

namespace
{
enum ModrinthSourceColumn
{
    SourceNameColumn = 0,
    SourceApiColumn,
    SourceWebsiteColumn,
};

QTreeWidgetItem* makeModrinthSourceItem(QTreeWidget* tree,
                                        const Modrinth::Source& source)
{
    auto* item = new QTreeWidgetItem(
        tree, {source.name, source.api_url, source.website_url});
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setIcon(SourceNameColumn, Modrinth::sourceIcon(source));
    return item;
}

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

// Mods are code, so remote sources have to use HTTPS. HTTP stays allowed for
// local ones, like a Labrinth running on this machine.
QString sanitizeModrinthSourceUrl(const QString& text)
{
    auto normalized = Modrinth::normalizeApiUrl(text);
    if (normalized.isEmpty())
    {
        return {};
    }

    QUrl url(normalized);
    if (url.scheme() == "http" && !AssetsUtils::isLocalMetadataHost(url.host()))
    {
        url.setScheme("https");
    }
    return url.toString();
}

bool isUsableModrinthSourceUrl(const QString& text)
{
    QUrl url(text);
    return url.isValid() && !url.host().isEmpty() &&
           (url.scheme() == "https" || url.scheme() == "http");
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
    connect(ui->removeModrinthSourceButton, &QPushButton::clicked, this,
            &APIPage::removeModrinthSource);
    connect(ui->testModrinthSourceButton, &QPushButton::clicked, this,
            &APIPage::testModrinthSource);
}

APIPage::~APIPage()
{
    // The test writes into m_modrinth_test_response
    stopModrinthTest();
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

    applyModrinthSources();

    APPLICATION->updateCapabilities();
}

void APIPage::loadModrinthSources()
{
    ui->modrinthSourcesTree->clear();
    for (const auto& source : Modrinth::customSources())
    {
        makeModrinthSourceItem(ui->modrinthSourcesTree, source);
    }
    for (int i = 0; i < ui->modrinthSourcesTree->columnCount(); i++)
    {
        ui->modrinthSourcesTree->resizeColumnToContents(i);
    }
}

bool APIPage::validateModrinthSources()
{
    QStringList problems;
    QStringList api_urls;
    QTreeWidgetItem* first_invalid = nullptr;

    for (int i = 0; i < ui->modrinthSourcesTree->topLevelItemCount(); i++)
    {
        auto* item = ui->modrinthSourcesTree->topLevelItem(i);
        if (!item)
        {
            continue;
        }

        auto name = item->text(SourceNameColumn).trimmed();
        auto label = name.isEmpty() ? tr("Source %1").arg(i + 1) : name;
        auto entered_api_url = item->text(SourceApiColumn).trimmed();
        auto api_url = sanitizeModrinthSourceUrl(entered_api_url);
        auto website = item->text(SourceWebsiteColumn).trimmed();

        QString problem;
        if (api_url.isEmpty())
        {
            problem = tr("%1 has no API URL.").arg(label);
        }
        else if (!isUsableModrinthSourceUrl(api_url))
        {
            problem = tr("The API URL of %1, \"%2\", isn't an http:// or "
                         "https:// address.")
                          .arg(label, entered_api_url);
        }
        else if (api_url ==
                 Modrinth::normalizeApiUrl(BuildConfig.MODRINTH_PROD_URL))
        {
            problem = tr("%1 is official Modrinth, which is always there.")
                          .arg(label);
        }
        else if (api_urls.contains(api_url))
        {
            problem =
                tr("%1 has the same API URL as another source.").arg(label);
        }
        else if (!website.isEmpty() &&
                 !isUsableModrinthSourceUrl(sanitizeModrinthSourceUrl(website)))
        {
            problem = tr("The website of %1, \"%2\", isn't an http:// or "
                         "https:// address.")
                          .arg(label, website);
        }

        api_urls.append(api_url);
        if (!problem.isEmpty())
        {
            problems.append(problem);
            if (!first_invalid)
            {
                first_invalid = item;
            }
        }
    }

    if (problems.isEmpty())
    {
        return true;
    }

    if (m_container)
    {
        m_container->selectPage(id());
    }
    ui->tabWidget->setCurrentWidget(ui->tab_modrinth);
    ui->modrinthSourcesTree->setCurrentItem(first_invalid);

    QMessageBox::warning(
        this, tr("Invalid Modrinth sources"),
        tr("These Modrinth sources can't be saved:\n\n%1\n\nFix or remove "
           "them first.")
            .arg(problems.join('\n')));
    return false;
}

void APIPage::applyModrinthSources()
{
    // validateModrinthSources() already made sure every row is usable
    QList<Modrinth::Source> sources;
    for (int i = 0; i < ui->modrinthSourcesTree->topLevelItemCount(); i++)
    {
        auto* item = ui->modrinthSourcesTree->topLevelItem(i);
        if (!item)
        {
            continue;
        }

        Modrinth::Source source;
        source.name = item->text(SourceNameColumn).trimmed();
        source.api_url = sanitizeModrinthSourceUrl(item->text(SourceApiColumn));
        source.website_url =
            sanitizeModrinthSourceUrl(item->text(SourceWebsiteColumn));
        sources.append(source);
    }

    auto old_sources = Modrinth::customSources();
    Modrinth::setCustomSources(sources);

    // New sites, or sites with another website, may have another icon
    for (const auto& source : std::as_const(sources))
    {
        if (!hasSameModrinthSite(old_sources, source))
        {
            Modrinth::fetchSourceIcon(source, true);
        }
    }
}

void APIPage::addModrinthSource()
{
    Modrinth::Source source;
    source.name = tr("New source");

    auto* item = makeModrinthSourceItem(ui->modrinthSourcesTree, source);
    // Without an API URL yet it would get official Modrinth's icon
    item->setIcon(SourceNameColumn, APPLICATION->getThemedIcon("worlds"));
    ui->modrinthSourcesTree->setCurrentItem(item);
    ui->modrinthSourcesTree->editItem(item, SourceNameColumn);
}

void APIPage::removeModrinthSource()
{
    delete ui->modrinthSourcesTree->currentItem();
}

void APIPage::testModrinthSource()
{
    auto* item = ui->modrinthSourcesTree->currentItem();
    if (!item)
    {
        ui->modrinthTestResult->setText(tr("Select a source to test it."));
        return;
    }

    auto api_url = sanitizeModrinthSourceUrl(item->text(SourceApiColumn));
    if (!isUsableModrinthSourceUrl(api_url))
    {
        ui->modrinthTestResult->setText(
            tr("The API URL has to be an http:// or https:// address."));
        return;
    }

    stopModrinthTest();
    m_modrinth_test_response.clear();

    m_modrinth_test_job =
        new NetJob(tr("Test Modrinth source"), APPLICATION->network());
    m_modrinth_test_job->addNetAction(Net::Download::makeByteArray(
        QUrl(api_url + "/search?limit=1"), &m_modrinth_test_response));
    connect(m_modrinth_test_job.get(), &NetJob::succeeded, this,
            &APIPage::onModrinthTestSucceeded);
    connect(m_modrinth_test_job.get(), &NetJob::failed, this,
            &APIPage::onModrinthTestFailed);

    ui->modrinthTestResult->setText(tr("Testing %1...").arg(api_url));
    m_modrinth_test_job->start();
}

void APIPage::onModrinthTestSucceeded()
{
    QJsonParseError parse_error{};
    auto doc = QJsonDocument::fromJson(m_modrinth_test_response, &parse_error);
    auto obj = doc.object();
    if (parse_error.error != QJsonParseError::NoError ||
        !obj.value("hits").isArray())
    {
        ui->modrinthTestResult->setText(
            tr("This doesn't look like a Modrinth API, searching it didn't "
               "return projects. Check that the URL is the versioned API "
               "root, ending in something like /v2."));
        return;
    }

    ui->modrinthTestResult->setText(
        tr("It works, the search found %n project(s).", nullptr,
           obj.value("total_hits").toInt()));
}

void APIPage::onModrinthTestFailed(QString reason)
{
    ui->modrinthTestResult->setText(tr("Couldn't reach it: %1").arg(reason));
}

void APIPage::stopModrinthTest()
{
    if (m_modrinth_test_job && m_modrinth_test_job->isRunning())
    {
        m_modrinth_test_job->disconnect(this);
        m_modrinth_test_job->abort();
    }
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
    // Saving would drop the broken sources, keep the dialog open instead
    if (!validateModrinthSources())
    {
        return false;
    }
    applySettings();
    return true;
}

void APIPage::retranslate()
{
    ui->retranslateUi(this);
}

#include "moc_APIPage.cpp"
