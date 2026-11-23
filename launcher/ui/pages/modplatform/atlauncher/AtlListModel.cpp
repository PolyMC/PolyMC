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

#include "AtlListModel.h"

#include <BuildConfig.h>
#include <Application.h>
#include <Json.h>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QUrl>

#include <utility>

#include "net/Download.h"
#include "ui/pages/modplatform/LogoCache.h"

namespace Atl
{

ListModel::ListModel(QObject* parent)
    : QAbstractListModel(parent),
      m_logos(new LogoCache("ATLauncherPacks", "ATLauncher Icon Download",
                            "atlauncher-placeholder", LogoCache::Skip::Nothing,
                            this))
{
    connect(m_logos, &LogoCache::logoLoaded, this, &ListModel::onLogoLoaded);
}

ListModel::~ListModel()
{
    if (jobPtr && jobPtr->isRunning())
    {
        jobPtr->disconnect(this);
        jobPtr->abort();
    }
}

int ListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return modpacks.size();
}

int ListModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant ListModel::data(const QModelIndex& index, int role) const
{
    int pos = index.row();
    if (pos >= modpacks.size() || pos < 0 || !index.isValid())
    {
        return QString("INVALID INDEX %1").arg(pos);
    }

    ATLauncher::IndexedPack pack = modpacks.at(pos);
    if (role == Qt::DisplayRole)
    {
        return pack.name;
    }
    else if (role == Qt::ToolTipRole)
    {
        return pack.name;
    }
    else if (role == Qt::DecorationRole)
    {
        auto url =
            (BuildConfig.ATL_DOWNLOAD_SERVER_URL + "launcher/images/%1.png")
                .arg(pack.safeName.toLower());
        return m_logos->icon(pack.safeName, url);
    }
    else if (role == Qt::UserRole)
    {
        QVariant v;
        v.setValue(pack);
        return v;
    }

    return QVariant();
}

void ListModel::request()
{
    beginResetModel();
    modpacks.clear();
    endResetModel();

    auto* netJob = new NetJob("Atl::Request", APPLICATION->network());
    auto url =
        BuildConfig.ATL_DOWNLOAD_SERVER_URL + "launcher/json/packsnew.json";
    netJob->addNetAction(Net::Download::makeByteArray(QUrl(url), &response));
    jobPtr = netJob;
    jobPtr->start();

    QObject::connect(netJob, &NetJob::succeeded, this,
                     &ListModel::requestFinished);
    QObject::connect(netJob, &NetJob::failed, this, &ListModel::requestFailed);
}

void ListModel::requestFinished()
{
    jobPtr.reset();

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from ATL at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << response;
        return;
    }

    QList<ATLauncher::IndexedPack> newList;

    auto packs = doc.array();
    for (auto packRaw : std::as_const(packs))
    {
        auto packObj = packRaw.toObject();

        ATLauncher::IndexedPack pack;

        try
        {
            ATLauncher::loadIndexedPack(pack, packObj);
        }
        catch (const JSONValidationError& e)
        {
            qDebug() << QString::fromUtf8(response);
            qWarning() << "Error while reading pack manifest from ATLauncher: "
                       << e.cause();
            return;
        }

        // ignore packs without a published version
        if (pack.versions.length() == 0)
        {
            continue;
        }
        // only display public packs (for now)
        if (pack.type != ATLauncher::PackType::Public)
        {
            continue;
        }
        // ignore "system" packs (Vanilla, Vanilla with Forge, etc)
        if (pack.system)
        {
            continue;
        }

        newList.append(pack);
    }

    // Inserting zero rows is invalid, and asserts in debug builds of Qt
    if (newList.isEmpty())
    {
        return;
    }

    beginInsertRows(QModelIndex(), modpacks.size(),
                    modpacks.size() + newList.size() - 1);
    modpacks.append(newList);
    endInsertRows();
}

void ListModel::requestFailed(QString /*reason*/)
{
    jobPtr.reset();
}

QString ListModel::logoPath(const QString& logo, const QString& url)
{
    return m_logos->path(logo, url);
}

void ListModel::onLogoLoaded(const QString& logo)
{
    for (int i = 0; i < modpacks.size(); i++)
    {
        if (modpacks[i].safeName == logo)
        {
            emit dataChanged(createIndex(i, 0), createIndex(i, 0),
                             {Qt::DecorationRole});
        }
    }
}

}  // namespace Atl

#include "moc_AtlListModel.cpp"
