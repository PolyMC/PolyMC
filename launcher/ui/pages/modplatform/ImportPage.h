/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QWidget>

#include "tasks/Task.h"
#include "ui/pages/BasePage.h"
#include <Application.h>

namespace Ui
{
class ImportPage;
}

class NewInstanceDialog;

class ImportPage : public QWidget, public BasePage
{
    Q_OBJECT

public:
    explicit ImportPage(NewInstanceDialog* dialog, QWidget *parent = nullptr);
    ~ImportPage() override;
    QString displayName() const override
    {
        return tr("Import from zip");
    }
    QIcon icon() const override
    {
        return APPLICATION->getThemedIcon("viewfolder");
    }
    QString id() const override
    {
        return "import";
    }
    QString helpPage() const override
    {
        return "Zip-import";
    }
    bool shouldDisplay() const override;

    void setUrl(const QString & url);
    void openedImpl() override;

private slots:
    void on_modpackBtn_clicked();
    void updateState();

private:
    QUrl modpackUrl() const;

private:
    Ui::ImportPage *ui = nullptr;
    NewInstanceDialog* dialog = nullptr;
};

