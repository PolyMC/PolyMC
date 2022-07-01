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

#include <QJsonObject>
#include <QJsonArray>

#include "Rule.h"

RuleAction RuleAction_fromString(QString name)
{
    if (name == "allow")
        return Allow;
    if (name == "disallow")
        return Disallow;
    return Defer;
}

QList<std::shared_ptr<Rule>> rulesFromJsonV4(const QJsonObject &objectWithRules)
{
    QList<std::shared_ptr<Rule>> rules;
    auto rulesVal = objectWithRules.value("rules");
    if (!rulesVal.isArray())
        return rules;

    QJsonArray ruleList = rulesVal.toArray();
    for (auto ruleVal : ruleList)
    {
        std::shared_ptr<Rule> rule;
        if (!ruleVal.isObject())
            continue;
        auto ruleObj = ruleVal.toObject();
        auto actionVal = ruleObj.value("action");
        if (!actionVal.isString())
            continue;
        auto action = RuleAction_fromString(actionVal.toString());
        if (action == Defer)
            continue;

        auto osVal = ruleObj.value("os");
        if (!osVal.isObject())
        {
            // add a new implicit action rule
            rules.append(ImplicitRule::create(action));
            continue;
        }

        auto osObj = osVal.toObject();
        auto osNameVal = osObj.value("name");
        if (!osNameVal.isString())
            continue;
        // add a new OS rule
        rules.append(OsRule::create(action, osNameVal.toString()));
    }
    return rules;
}

QJsonObject ImplicitRule::toJson()
{
    QJsonObject ruleObj;
    ruleObj.insert("action", m_result == Allow ? QString("allow") : QString("disallow"));
    return ruleObj;
}

QJsonObject OsRule::toJson()
{
    QJsonObject ruleObj;
    ruleObj.insert("action", m_result == Allow ? QString("allow") : QString("disallow"));
    QJsonObject osObj;
    if(!m_system.isEmpty()) {
        osObj.insert("name", m_system);
    }

    ruleObj.insert("os", osObj);
    return ruleObj;
}

bool OsRule::applies(Library *parent, const SettingsObjectPtr& settingsObjJavaArch)
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
        // if the arch isn't empty then it has to be archDependent right?
        parent->setArchDependent(true);
        archCorrect = arch == SysInfo::currentArch(settingsObjJavaArch);
    }
    qDebug() << "Os rule with OS required" << m_system << systemCorrect << "Arch required" << arch << archCorrect;
    return systemCorrect && archCorrect;
}

