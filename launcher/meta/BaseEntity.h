/* Copyright 2015-2021 MultiMC Contributors
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

#include <QJsonObject>
#include <QString>
#include <QUrl>
#include <memory>

#include "net/Mode.h"
#include "net/NetJob.h"
#include "tasks/Task.h"

namespace Meta
{
class BaseEntity
{
public: /* types */
    using Ptr = std::shared_ptr<BaseEntity>;
    enum class LoadStatus
    {
        NotLoaded,
        Local,
        Remote
    };

public:
    virtual ~BaseEntity();

    virtual void parse(const QJsonObject& obj) = 0;
    void parseRemote(const QJsonObject& obj, const QByteArray& data);

    virtual QString localFilename() const = 0;
    virtual QUrl url() const;

    bool isLoaded() const;
    // False when the index that lists this entity expects a different file
    bool isUpToDate() const;
    bool shouldStartRemoteUpdate() const;

    void load(Net::Mode loadType);
    Task::Ptr getCurrentTask();

public: /* for parsers */
    void setSha256(const QString& sha256);

protected: /* methods */
    bool loadLocalFile();

protected: /* data */
    // The SHA-256 the parent index lists, and that of the loaded file
    QString m_sha256;
    QString m_fileSha256;

private:
    LoadStatus m_loadStatus = LoadStatus::NotLoaded;
    NetJob::Ptr m_updateTask;
};
}  // namespace Meta
