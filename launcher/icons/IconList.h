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

#include <QMutex>
#include <QAbstractListModel>
#include <QFile>
#include <QDir>
#include <QtGui/QIcon>
#include <memory>

#include "MMCIcon.h"
#include "settings/Setting.h"

#include "QObjectPtr.h"

class QFileSystemWatcher;

class IconList : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit IconList(const QStringList &builtinPaths, QString path, QObject *parent = nullptr);
    ~IconList() override {};

    QIcon getIcon(const QString &key) const;
    int getIconIndex(const QString &key) const;
    QString getDirectory() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QStringList mimeTypes() const override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool addThemeIcon(const QString &key);
    bool addIcon(const QString &key, const QString &name, const QString &path, const IconType type);
    void saveIcon(const QString &key, const QString &path, const char * format) const;
    bool deleteIcon(const QString &key);
    bool iconFileExists(const QString &key) const;

    void installIcons(const QStringList &iconFiles);
    void installIcon(const QString &file, const QString &name);

    const MMCIcon * icon(const QString &key) const;

    void startWatching();
    void stopWatching();
    // hide copy constructor
    IconList(const IconList &) = delete;
    // hide assign op
    IconList &operator=(const IconList &) = delete;

signals:
    void iconUpdated(QString key);

private:
    void reindex();

public slots:
    void directoryChanged(const QString &path);

protected slots:
    void fileChanged(const QString &path);
    void SettingChanged(const Setting & setting, QVariant value);
private:
    shared_qobject_ptr<QFileSystemWatcher> m_watcher;
    bool is_watching;
    QMap<QString, int> name_index;
    QVector<MMCIcon> icons;
    QDir m_dir;
};
