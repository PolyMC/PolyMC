// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
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
#include <Json.hpp>

#include "Rule.h"

RuleAction RuleAction_fromString(QString name)
{
    if (name == "allow")
        return Allow;
    if (name == "disallow")
        return Disallow;
    return Defer;
}

QList<std::shared_ptr<Rule>> rulesFromJsonV4(const nlohmann::json &objectWithRules)
{
    QList<std::shared_ptr<Rule>> rules;
    auto rulesVal = objectWithRules["rules"];
    if (!rulesVal.is_array())
        return rules;

    for (auto ruleVal : rulesVal)
    {
        std::shared_ptr<Rule> rule;
        if (!ruleVal.is_object())
            continue;
        auto actionVal = ruleVal["action"];
        if (!actionVal.is_string())
            continue;
        auto action = RuleAction_fromString(QString::fromStdString(actionVal.get<std::string>()));
        if (action == Defer)
            continue;

        auto osVal = ruleVal["os"];
        if (!osVal.is_object())
        {
            // add a new implicit action rule
            rules.append(ImplicitRule::create(action));
            continue;
        }

        auto osNameVal = osVal["name"];
        if (!osNameVal.is_string())
            continue;
        QString osName = QString::fromStdString(osNameVal.get<std::string>());

        //qDebug() << "osVal: " << osVal.dump(4).c_str();
        //QString versionRegex = QString::fromStdString(osVal["version"].get<std::string>());
        QString versionRegex = QString::fromStdString(osVal.value("version", ""));
        // add a new OS rule
        rules.append(OsRule::create(action, osName, versionRegex));
    }
    return rules;
}

nlohmann::json ImplicitRule::toJson()
{
    /*
    QJsonObject ruleObj;
    ruleObj.insert("action", m_result == Allow ? QString("allow") : QString("disallow"));
    return ruleObj;
        */
    nlohmann::json ruleObj;
    ruleObj["action"] = m_result == Allow ? "allow" : "disallow";
    return ruleObj;
}

nlohmann::json OsRule::toJson()
{
        /*
    QJsonObject ruleObj;
    ruleObj.insert("action", m_result == Allow ? QString("allow") : QString("disallow"));
    QJsonObject osObj;
    {
        osObj.insert("name", m_system);
        if(!m_version_regexp.isEmpty())
        {
            osObj.insert("version", m_version_regexp);
        }
    }
    ruleObj.insert("os", osObj);
    return ruleObj;
        */
   nlohmann::json ruleObj;
   ruleObj["action"] = m_result == Allow ? "allow" : "disallow";
   nlohmann::json osObj;
   {
       osObj["name"] = m_system.toStdString();
       if(!m_version_regexp.isEmpty())
       {
           osObj["version"] = m_version_regexp.toStdString();
       }
   }
   ruleObj["os"] = osObj;
   return ruleObj;
}

