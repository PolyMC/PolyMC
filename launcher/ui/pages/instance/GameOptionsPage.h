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

#include <QString>
#include <QWidget>

#include "ui/pages/BasePage.h"
#include <Application.h>

namespace Ui
{
class GameOptionsPage;
}

class GameOptions;
class MinecraftInstance;

class GameOptionsPage : public QWidget, public BasePage
{
    Q_OBJECT

public:
    explicit GameOptionsPage(MinecraftInstance *inst, QWidget *parent = nullptr);
    ~GameOptionsPage() override;

    void openedImpl() override;
    void closedImpl() override;

    QString displayName() const override
    {
        return tr("Game Options");
    }
    QIcon icon() const override
    {
        return APPLICATION->getThemedIcon("settings");
    }
    QString id() const override
    {
        return "gameoptions";
    }
    QString helpPage() const override
    {
        return "Game-Options-management";
    }

private: // data
    Ui::GameOptionsPage *ui = nullptr;
    std::shared_ptr<GameOptions> m_model;
};
