// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 Sefa Eyeoglu <contact@scrumplex.net>
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

#include "FlameModPage.h"
#include "ui_FlameModPage.h"

#include <QKeyEvent>

#include "Application.h"
#include "FlameModModel.h"
#include "InstanceImportTask.h"
#include "Json.h"
#include "ModDownloadTask.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "ui/dialogs/ModDownloadDialog.h"

FlameModPage::FlameModPage(ModDownloadDialog *dialog, BaseInstance *instance)
    : QWidget(dialog), m_instance(instance), ui(new Ui::FlameModPage),
      dialog(dialog) {
  ui->setupUi(this);
  connect(ui->searchButton, &QPushButton::clicked, this,
          &FlameModPage::triggerSearch);
  ui->searchEdit->installEventFilter(this);
  listModel = new FlameMod::ListModel(this);
  ui->packView->setModel(listModel);

  ui->versionSelectionBox->view()->setVerticalScrollBarPolicy(
      Qt::ScrollBarAsNeeded);
  ui->versionSelectionBox->view()->parentWidget()->setMaximumHeight(300);

  // index is used to set the sorting with the flame api
  ui->sortByBox->addItem(tr("Sort by Featured"));
  ui->sortByBox->addItem(tr("Sort by Popularity"));
  ui->sortByBox->addItem(tr("Sort by last updated"));
  ui->sortByBox->addItem(tr("Sort by Name"));
  ui->sortByBox->addItem(tr("Sort by Author"));
  ui->sortByBox->addItem(tr("Sort by Downloads"));

  connect(ui->sortByBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(triggerSearch()));
  connect(ui->packView->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &FlameModPage::onSelectionChanged);
  connect(ui->versionSelectionBox, &QComboBox::currentTextChanged, this,
          &FlameModPage::onVersionSelectionChanged);
  connect(ui->modSelectionButton, &QPushButton::clicked, this,
          &FlameModPage::onModSelected);
}

FlameModPage::~FlameModPage() { delete ui; }

bool FlameModPage::eventFilter(QObject *watched, QEvent *event) {
  if (watched == ui->searchEdit && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Return) {
      triggerSearch();
      keyEvent->accept();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool FlameModPage::shouldDisplay() const { return true; }

void FlameModPage::retranslate()
{
    ui->retranslateUi(this);
}

void FlameModPage::openedImpl() {
  updateSelectionButton();
  triggerSearch();
}

void FlameModPage::triggerSearch() {
  listModel->searchWithTerm(ui->searchEdit->text(),
                            ui->sortByBox->currentIndex());
}

void FlameModPage::onSelectionChanged(QModelIndex first, QModelIndex second) {
  ui->versionSelectionBox->clear();

  if (!first.isValid()) {
    return;
  }

  current = listModel->data(first, Qt::UserRole).value<FlameMod::IndexedPack>();
  QString text = "";
  QString name = current.name;

  if (current.websiteUrl.isEmpty())
    text = name;
  else
    text = "<a href=\"" + current.websiteUrl + "\">" + name + "</a>";
  if (!current.authors.empty()) {
    auto authorToStr = [](FlameMod::ModpackAuthor &author) {
      if (author.url.isEmpty()) {
        return author.name;
      }
      return QString("<a href=\"%1\">%2</a>").arg(author.url, author.name);
    };
    QStringList authorStrs;
    for (auto &author : current.authors) {
      authorStrs.push_back(authorToStr(author));
    }
    text += "<br>" + tr(" by ") + authorStrs.join(", ");
  }
  text += "<br><br>";

  ui->packDescription->setHtml(text + current.description);

  if (!current.versionsLoaded) {
    qDebug() << "Loading flame mod versions";

    ui->modSelectionButton->setText(tr("Loading versions..."));
    ui->modSelectionButton->setEnabled(false);

    auto netJob =
        new NetJob(QString("Flame::ModVersions(%1)").arg(current.name),
                   APPLICATION->network());
    auto response = new QByteArray();
    int addonId = current.addonId;
    netJob->addNetAction(Net::Download::makeByteArray(
        QString("https://addons-ecs.forgesvc.net/api/v2/addon/%1/files")
            .arg(addonId),
        response));

    QObject::connect(netJob, &NetJob::succeeded, this, [this, response, addonId] {
        if(addonId != current.addonId){
            return; //wrong request
        }
      QJsonParseError parse_error;
      QJsonDocument doc = QJsonDocument::fromJson(*response, &parse_error);
      if (parse_error.error != QJsonParseError::NoError) {
        qWarning() << "Error while parsing JSON response from Flame at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << *response;
        return;
      }
      QJsonArray arr = doc.array();
      try {
        FlameMod::loadIndexedPackVersions(current, arr, APPLICATION->network(),
                                          m_instance);
      } catch (const JSONValidationError &e) {
        qDebug() << *response;
        qWarning() << "Error while reading Flame mod version: " << e.cause();
      }
      auto packProfile = ((MinecraftInstance *)m_instance)->getPackProfile();
      QString mcVersion = packProfile->getComponentVersion("net.minecraft");
      QString loaderString =
          (packProfile->getComponentVersion("net.minecraftforge").isEmpty())
              ? "fabric"
              : "forge";
      for (int i = 0; i < current.versions.size(); i++) {
        auto version = current.versions[i];
        if (!version.mcVersion.contains(mcVersion)) {
          continue;
        }
        ui->versionSelectionBox->addItem(version.version, QVariant(i));
      }
      if (ui->versionSelectionBox->count() == 0) {
        ui->versionSelectionBox->addItem(tr("No valid version found."),
                                         QVariant(-1));
      }

      ui->modSelectionButton->setText(tr("Cannot select invalid version :("));
      updateSelectionButton();
    });
    QObject::connect(netJob, &NetJob::finished, this, [response, netJob] {
      netJob->deleteLater();
      delete response;
    });
    netJob->start();
  } else {
    for (int i = 0; i < current.versions.size(); i++) {
      ui->versionSelectionBox->addItem(current.versions[i].version,
                                       QVariant(i));
    }
    if (ui->versionSelectionBox->count() == 0) {
      ui->versionSelectionBox->addItem(tr("No valid version found."),
                                       QVariant(-1));
    }

    updateSelectionButton();
  }
}

void FlameModPage::updateSelectionButton() {
  if (!isOpened || selectedVersion < 0) {
    ui->modSelectionButton->setEnabled(false);
    return;
  }

  ui->modSelectionButton->setEnabled(true);
  auto &version = current.versions[selectedVersion];
  if (!dialog->isModSelected(current.name, version.fileName)) {
    ui->modSelectionButton->setText(tr("Select mod for download"));
  } else {
    ui->modSelectionButton->setText(tr("Deselect mod for download"));
  }
}

void FlameModPage::onVersionSelectionChanged(QString data) {
  if (data.isNull() || data.isEmpty()) {
    selectedVersion = -1;
    return;
  }
  selectedVersion = ui->versionSelectionBox->currentData().toInt();
  updateSelectionButton();
}

void FlameModPage::onModSelected() {
  auto &version = current.versions[selectedVersion];
  if (dialog->isModSelected(current.name, version.fileName)) {
    dialog->removeSelectedMod(current.name);
  } else {
    dialog->addSelectedMod(current.name,
                           new ModDownloadTask(version.downloadUrl,
                                               version.fileName, dialog->mods));
  }

  updateSelectionButton();
}
