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

#include "minecraft/auth/AuthSession.h"
#include "minecraft/launch/MinecraftServerTarget.h"
#include <launch/LaunchStep.h>
#include <memory>
#include <utility>

// FIXME: temporary wrapper for existing task.
class PrintInstanceInfo: public LaunchStep
{
    Q_OBJECT
public:
    explicit PrintInstanceInfo(LaunchTask *parent, AuthSessionPtr session, MinecraftServerTargetPtr serverToJoin) :
        LaunchStep(parent), m_session(std::move(std::move(session))), m_serverToJoin(std::move(std::move(serverToJoin))) {};
    ~PrintInstanceInfo() override = default;

    void executeTask() override;
    bool canAbort() const override
    {
        return false;
    }
private:
    AuthSessionPtr m_session;
    MinecraftServerTargetPtr m_serverToJoin;
};

