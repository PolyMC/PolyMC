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
#include <QList>
#include <QJsonObject>
#include <memory>
#include "OpSys.h"

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
    virtual bool applies(const Library *parent) = 0;

public:
    Rule(RuleAction result) : m_result(result)
    {
    }
    virtual ~Rule() {};
    virtual QJsonObject toJson() = 0;
    RuleAction apply(const Library *parent)
    {
        if (applies(parent))
            return m_result;
        else
            return Defer;
    }
};

class OsRule : public Rule
{
private:
    // the OS
    QString m_system;
    // arch
    QString m_arch;

    QStringList x86_arches = { "x86_64", "i386" };
    QStringList arm_arches = { "arm64", "arm", "armhf" };

protected:
    virtual bool applies(const Library *)
    {
        bool systemCorrect;
        if(m_system.isEmpty())
        {
            systemCorrect = true;
        }
        else
        {
            auto sys = OpSys_fromString(m_system);
            systemCorrect = sys == currentSystem;
        }
        auto cpuArch = QSysInfo::currentCpuArchitecture();
        bool archCorrect;
        if(m_arch == "arm_generic")
        {
            archCorrect = arm_arches.contains(cpuArch);
        }
        else if(m_arch == "x86_generic")
        {
            archCorrect = x86_arches.contains(cpuArch);
        }
        else if(!m_arch.isEmpty()){
            archCorrect = m_arch == cpuArch;
        }
        // if m_arch is empty, don't compare, or it will break
        else
        {
            archCorrect = true;
        }
        qDebug() << "Os rule with OS required" << m_system << "and arch required" << m_arch << "with" << systemCorrect << archCorrect;
        return systemCorrect && archCorrect;
    }
OsRule(RuleAction result, QString system, QString arch)
        : Rule(result), m_system(system), m_arch(arch)
    {
    }

public:
    virtual QJsonObject toJson();
    static std::shared_ptr<OsRule> create(RuleAction result, QString system, QString arch)
    {
        return std::shared_ptr<OsRule>(new OsRule(result, system, arch));
    }
};

class ImplicitRule : public Rule
{
protected:
    virtual bool applies(const Library *)
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
