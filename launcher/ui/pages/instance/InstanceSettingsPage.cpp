// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
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

#include "InstanceSettingsPage.h"
#include "ui_InstanceSettingsPage.h"

#include <QFileDialog>
#include <QDialog>
#include <QMessageBox>

#include <sys.h>

#include "ui/dialogs/VersionSelectDialog.h"
#include "ui/widgets/CustomCommands.h"

#include "JavaCommon.h"
#include "Application.h"

#include "java/JavaInstallList.h"
#include "FileSystem.h"


InstanceSettingsPage::InstanceSettingsPage(BaseInstance *inst, QWidget *parent)
    : QWidget(parent), ui(new Ui::InstanceSettingsPage), m_instance(inst)
{
    m_settings = inst->settings();
    ui->setupUi(this);
    auto sysMB = Sys::getSystemRam() / Sys::mebibyte;
    ui->maxMemSpinBox->setMaximum(sysMB);
    connect(ui->openGlobalJavaSettingsButton, &QCommandLinkButton::clicked, this, &InstanceSettingsPage::globalSettingsButtonClicked);
    connect(APPLICATION, &Application::globalSettingsAboutToOpen, this, &InstanceSettingsPage::applySettings);
    connect(APPLICATION, &Application::globalSettingsClosed, this, &InstanceSettingsPage::loadSettings);
    loadSettings();
}

bool InstanceSettingsPage::shouldDisplay() const
{
    return !m_instance->isRunning();
}

InstanceSettingsPage::~InstanceSettingsPage()
{
    delete ui;
}

void InstanceSettingsPage::globalSettingsButtonClicked(bool)
{
    switch(ui->settingsTabs->currentIndex()) {
        case 0:
            APPLICATION->ShowGlobalSettings(this, "java-settings");
            return;
        case 1:
            APPLICATION->ShowGlobalSettings(this, "minecraft-settings");
            return;
        case 2:
            APPLICATION->ShowGlobalSettings(this, "custom-commands");
            return;
    }
}

bool InstanceSettingsPage::apply()
{
    applySettings();
    return true;
}

void InstanceSettingsPage::applySettings()
{
    SettingsObject::Lock lock(m_settings);

    // Console
    bool console = ui->consoleSettingsBox->isChecked();
    m_settings->set("OverrideConsole", console);
    m_settings->set("ShowConsole", ui->showConsoleCheck->isChecked());
    m_settings->set("AutoCloseConsole", ui->autoCloseConsoleCheck->isChecked());
    m_settings->set("ShowConsoleOnError", ui->showConsoleErrorCheck->isChecked());

    // Window Size
    bool window = ui->windowSizeGroupBox->isChecked();
    m_settings->set("OverrideWindow", window);
    m_settings->set("LaunchMaximized", ui->maximizedCheckBox->isChecked());
    m_settings->set("MinecraftWinWidth", ui->windowWidthSpinBox->value());
    m_settings->set("MinecraftWinHeight", ui->windowHeightSpinBox->value());

    // Memory
    bool memory = ui->memoryGroupBox->isChecked();
    m_settings->set("OverrideMemory", memory);
    int min = ui->minMemSpinBox->value();
    int max = ui->maxMemSpinBox->value();
    if(min < max)
    {
        m_settings->set("MinMemAlloc", min);
        m_settings->set("MaxMemAlloc", max);
    }
    else
    {
        m_settings->set("MinMemAlloc", max);
        m_settings->set("MaxMemAlloc", min);
    }
    m_settings->set("PermGen", ui->permGenSpinBox->value());

    // Java Install Settings
    bool javaInstall = ui->javaSettingsGroupBox->isChecked();
    m_settings->set("OverrideJavaLocation", javaInstall);
    m_settings->set("JavaPath", ui->javaPathTextBox->text());
    m_settings->set("IgnoreJavaCompatibility", ui->skipCompatibilityCheckbox->isChecked());

    // Java arguments
    bool javaArgs = ui->javaArgumentsGroupBox->isChecked();
    m_settings->set("OverrideJavaArgs", javaArgs);
    m_settings->set("JvmArgs", ui->jvmArgsTextBox->toPlainText().replace("\n", " "));

    // old generic 'override both' is removed.
    m_settings->reset("OverrideJava");

    // Custom Commands
    bool custcmd = ui->customCommands->checked();
    m_settings->set("OverrideCommands", custcmd);
    m_settings->set("PreLaunchCommand", ui->customCommands->prelaunchCommand());
    m_settings->set("WrapperCommand", ui->customCommands->wrapperCommand());
    m_settings->set("PostExitCommand", ui->customCommands->postexitCommand());

    // Workarounds
    bool workarounds = ui->nativeWorkaroundsGroupBox->isChecked();
    m_settings->set("OverrideNativeWorkarounds", workarounds);
    m_settings->set("UseNativeOpenAL", ui->useNativeOpenALCheck->isChecked());
    m_settings->set("UseNativeGLFW", ui->useNativeGLFWCheck->isChecked());

    // Game time
    bool gameTime = ui->gameTimeGroupBox->isChecked();
    m_settings->set("OverrideGameTime", gameTime);
    m_settings->set("ShowGameTime", ui->showGameTime->isChecked());
    m_settings->set("RecordGameTime", ui->recordGameTime->isChecked());

    // Join server on launch
    bool joinServerOnLaunch = ui->serverJoinGroupBox->isChecked();
    m_settings->set("JoinServerOnLaunch", joinServerOnLaunch);
    m_settings->set("JoinServerOnLaunchAddress", ui->serverJoinAddress->text());
}

void InstanceSettingsPage::loadSettings()
{
    // Console
    ui->consoleSettingsBox->setChecked(m_settings->getStored("OverrideConsole").toBool());
    ui->showConsoleCheck->setChecked(m_settings->getStored("ShowConsole").toBool());
    ui->autoCloseConsoleCheck->setChecked(m_settings->getStored("AutoCloseConsole").toBool());
    ui->showConsoleErrorCheck->setChecked(m_settings->getStored("ShowConsoleOnError").toBool());

    // Window Size
    ui->windowSizeGroupBox->setChecked(m_settings->getStored("OverrideWindow").toBool());
    ui->maximizedCheckBox->setChecked(m_settings->getStored("LaunchMaximized").toBool());
    ui->windowWidthSpinBox->setValue(m_settings->getStored("MinecraftWinWidth").toInt());
    ui->windowHeightSpinBox->setValue(m_settings->getStored("MinecraftWinHeight").toInt());

    // Memory
    ui->memoryGroupBox->setChecked(m_settings->getStored("OverrideMemory").toBool());
    int min = m_settings->getStored("MinMemAlloc").toInt();
    int max = m_settings->getStored("MaxMemAlloc").toInt();
    if(min < max)
    {
        ui->minMemSpinBox->setValue(min);
        ui->maxMemSpinBox->setValue(max);
    }
    else
    {
        ui->minMemSpinBox->setValue(max);
        ui->maxMemSpinBox->setValue(min);
    }
    ui->permGenSpinBox->setValue(m_settings->getStored("PermGen").toInt());
    bool permGenVisible = m_settings->getStored("PermGenVisible").toBool();
    ui->permGenSpinBox->setVisible(permGenVisible);
    ui->labelPermGen->setVisible(permGenVisible);
    ui->labelPermgenNote->setVisible(permGenVisible);


    // Java Settings
    bool overrideJava = m_settings->getStored("OverrideJava").toBool();
    bool overrideLocation = m_settings->getStored("OverrideJavaLocation").toBool() || overrideJava;
    bool overrideArgs = m_settings->getStored("OverrideJavaArgs").toBool() || overrideJava;

    ui->javaSettingsGroupBox->setChecked(overrideLocation);
    ui->javaPathTextBox->setText(m_settings->getStored("JavaPath").toString());
    ui->skipCompatibilityCheckbox->setChecked(m_settings->getStored("IgnoreJavaCompatibility").toBool());

    ui->javaArgumentsGroupBox->setChecked(overrideArgs);
    ui->jvmArgsTextBox->setPlainText(m_settings->getStored("JvmArgs").toString());

    // Custom commands
    ui->customCommands->initialize(
        true,
        m_settings->getStored("OverrideCommands").toBool(),
        m_settings->getStored("PreLaunchCommand").toString(),
        m_settings->getStored("WrapperCommand").toString(),
        m_settings->getStored("PostExitCommand").toString()
    );

    // Workarounds
    ui->nativeWorkaroundsGroupBox->setChecked(m_settings->getStored("OverrideNativeWorkarounds").toBool());
    ui->useNativeGLFWCheck->setChecked(m_settings->getStored("UseNativeGLFW").toBool());
    ui->useNativeOpenALCheck->setChecked(m_settings->getStored("UseNativeOpenAL").toBool());

    // Miscellanous
    ui->gameTimeGroupBox->setChecked(m_settings->getStored("OverrideGameTime").toBool());
    ui->showGameTime->setChecked(m_settings->getStored("ShowGameTime").toBool());
    ui->recordGameTime->setChecked(m_settings->getStored("RecordGameTime").toBool());

    ui->serverJoinGroupBox->setChecked(m_settings->getStored("JoinServerOnLaunch").toBool());
    ui->serverJoinAddress->setText(m_settings->getStored("JoinServerOnLaunchAddress").toString());
}

void InstanceSettingsPage::on_javaDetectBtn_clicked()
{
    JavaInstallPtr java;

    VersionSelectDialog vselect(APPLICATION->javalist().get(), tr("Select a Java version"), this, true);
    vselect.setResizeOn(2);
    vselect.exec();

    if (vselect.result() == QDialog::Accepted && vselect.selectedVersion())
    {
        java = std::dynamic_pointer_cast<JavaInstall>(vselect.selectedVersion());
        ui->javaPathTextBox->setText(java->path);
        bool visible = java->id.requiresPermGen() && m_settings->getStored("OverrideMemory").toBool();
        ui->permGenSpinBox->setVisible(visible);
        ui->labelPermGen->setVisible(visible);
        ui->labelPermgenNote->setVisible(visible);
        m_settings->set("PermGenVisible", visible);
    }
}

void InstanceSettingsPage::on_javaBrowseBtn_clicked()
{
    QString raw_path = QFileDialog::getOpenFileName(this, tr("Find Java executable"));

    // do not allow current dir - it's dirty. Do not allow dirs that don't exist
    if(raw_path.isEmpty())
    {
        return;
    }
    QString cooked_path = FS::NormalizePath(raw_path);

    QFileInfo javaInfo(cooked_path);
    if(!javaInfo.exists() || !javaInfo.isExecutable())
    {
        return;
    }
    ui->javaPathTextBox->setText(cooked_path);

    // custom Java could be anything... enable perm gen option
    ui->permGenSpinBox->setVisible(true);
    ui->labelPermGen->setVisible(true);
    ui->labelPermgenNote->setVisible(true);
    m_settings->set("PermGenVisible", true);
}

void InstanceSettingsPage::on_javaTestBtn_clicked()
{
    if(checker)
    {
        return;
    }
    checker.reset(new JavaCommon::TestCheck(
        this, ui->javaPathTextBox->text(), ui->jvmArgsTextBox->toPlainText().replace("\n", " "),
        ui->minMemSpinBox->value(), ui->maxMemSpinBox->value(), ui->permGenSpinBox->value()));
    connect(checker.get(), SIGNAL(finished()), SLOT(checkerFinished()));
    checker->run();
}

void InstanceSettingsPage::checkerFinished()
{
    checker.reset();
}

void InstanceSettingsPage::retranslate()
{
    ui->retranslateUi(this);
    ui->customCommands->retranslate();  // TODO: why is this seperate from the others?
}
