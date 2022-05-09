// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 Kenneth Chew <kenneth.c0@protonmail.com>
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

#include "OverrideSetting.h"

OverrideSetting::OverrideSetting(std::shared_ptr<Setting> other, std::shared_ptr<Setting> gate)
    : Setting(other->configKeys(), QVariant())
{
    Q_ASSERT(other);
    Q_ASSERT(gate);
    m_other = other;
    m_gate = gate;
}

bool OverrideSetting::isOverriding() const
{
    return m_gate->get().toBool();
}

QVariant OverrideSetting::defValue() const
{
    return m_other->get();
}

QVariant OverrideSetting::get() const
{
    if(isOverriding())
    {
        return Setting::get();
    }
    return m_other->get();
}

QVariant OverrideSetting::getStored() const
{
    return Setting::get();
}

void OverrideSetting::reset()
{
    Setting::reset();
}

void OverrideSetting::set(QVariant value)
{
    Setting::set(value);
}
