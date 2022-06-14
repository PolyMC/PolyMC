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

// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 dada513 <dada513@protonmail.com>
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
#pragma once

#include <QString>
#include <QList>
#include <QJsonObject>
#include <memory>
#include "SysInfo.h"

class Library;
class Rule;

enum RuleAction
{
    Allow,
    Disallow,
    Defer
};

QList<std::shared_ptr<Rule>> rulesFromJsonV4(const QJsonObject &objectWithRules);

class Rule
{
protected:
    RuleAction m_result;
    virtual bool applies(const Library *parent, const SettingsObjectPtr& settingsObjJavaArch) = 0;

public:
    Rule(RuleAction result) : m_result(result)
    {
    }
    virtual ~Rule() {};
    virtual QJsonObject toJson() = 0;
    RuleAction apply(const Library *parent, const SettingsObjectPtr& settingsObjJavaArch)
    {
        if (applies(parent, settingsObjJavaArch))
            return m_result;
        else
            return Defer;
    }
};

class OsRule : public Rule
{
private:
    QString m_system;

protected:
    virtual bool applies(const Library *, const SettingsObjectPtr& settingsObjJavaArch)
    {
        QString sys;
        QString arch;
        if(m_system.contains("-"))
        {
            auto parts = m_system.split("-");
            sys = parts[0];
            arch = parts[1];
        }
        else
        {
            sys = m_system;
        }
        bool systemCorrect;
        bool archCorrect = true;
        systemCorrect = sys == SysInfo::currentSystem();
        if(!arch.isEmpty())
        {
            archCorrect = arch == SysInfo::currentArch(settingsObjJavaArch);
        }
        // qDebug() << "Os rule with OS required" << m_system << systemCorrect << "Arch required" << arch << archCorrect;
        return systemCorrect && archCorrect;
    }
OsRule(RuleAction result, QString system)
        : Rule(result), m_system(system)
    {
    }

public:
    virtual QJsonObject toJson();
    static std::shared_ptr<OsRule> create(RuleAction result, QString system)
    {
        return std::shared_ptr<OsRule>(new OsRule(result, system));
    }
};

class ImplicitRule : public Rule
{
protected:
    virtual bool applies(const Library *, const SettingsObjectPtr& settingsObjJavaArch)
    {
        return true;
    }
    ImplicitRule(RuleAction result) : Rule(result)
    {
    }

public:
    virtual QJsonObject toJson();
    static std::shared_ptr<ImplicitRule> create(RuleAction result)
    {
        return std::shared_ptr<ImplicitRule>(new ImplicitRule(result));
    }
};
