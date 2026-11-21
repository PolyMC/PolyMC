/*
 * Copyright 2020-2021 Jamie Mansfield <jmansfield@cadixdev.org>
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

#include <QAbstractListModel>

#include "net/NetJob.h"
#include <QIcon>
#include <modplatform/atlauncher/ATLPackIndex.h>

class LogoCache;

namespace Atl
{

class ListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ListModel(QObject* parent);
    virtual ~ListModel();

    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    void request();

    QString logoPath(const QString& logo, const QString& url);

private slots:
    void requestFinished();
    void requestFailed(QString reason);

private:
    void onLogoLoaded(const QString& logo);

private:
    QList<ATLauncher::IndexedPack> modpacks;

    LogoCache* m_logos;

    NetJob::Ptr jobPtr;
    QByteArray response;
};

}  // namespace Atl
