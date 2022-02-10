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

#include <QDnsLookup>
#include <QObjectPtr.h>
#include <launch/LaunchStep.h>

#include "minecraft/launch/MinecraftServerTarget.h"

class LookupServerAddress: public LaunchStep {
Q_OBJECT
public:
    explicit LookupServerAddress(LaunchTask *parent);
    ~LookupServerAddress() override = default;

    void executeTask() override;
    bool abort() override;
    bool canAbort() const override
    {
        return true;
    }

    void setLookupAddress(const QString &lookupAddress);
    void setOutputAddressPtr(MinecraftServerTargetPtr output);

private slots:
    void on_dnsLookupFinished();

private:
    void resolve(const QString &address, quint16 port);

    QDnsLookup *m_dnsLookup;
    QString m_lookupAddress;
    MinecraftServerTargetPtr m_output;
};
