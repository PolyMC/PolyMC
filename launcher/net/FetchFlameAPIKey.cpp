// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 Lenny McLennington <lenny@sneed.church>
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
 */

#include "FetchFlameAPIKey.h"
#include "Application.h"
#include <BuildConfig.h>
#include "json.hpp"

#include <ui/dialogs/ProgressDialog.h>

FetchFlameAPIKey::FetchFlameAPIKey(QObject *parent)
    : Task{parent}
{

}

void FetchFlameAPIKey::executeTask()
{
    QNetworkRequest req(BuildConfig.FLAME_API_KEY_API_URL);
    m_reply.reset(APPLICATION->network()->get(req));
    connect(m_reply.get(), &QNetworkReply::downloadProgress, this, &Task::setProgress);
    connect(m_reply.get(), &QNetworkReply::finished, this, &FetchFlameAPIKey::downloadFinished);
    connect(m_reply.get(),
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            &QNetworkReply::errorOccurred,
#else
            qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error),
#endif
            this,
            [this] (QNetworkReply::NetworkError error) {
                qCritical() << "Network error: " << error;
                emitFailed(m_reply->errorString());
            });

    setStatus(tr("Fetching Curseforge core API key"));
}

void FetchFlameAPIKey::downloadFinished()
{
    auto res = m_reply->readAll();
    nlohmann::json doc;
    try {
        doc = nlohmann::json::parse(res.constData(), res.constData() + res.size());
    }
    catch (nlohmann::json::parse_error& e) {
        qCritical() << "Failed to parse JSON: " << e.what();
        emitFailed("Failed to parse JSON");
        return;
    }

    //qDebug() << doc.dump(4).c_str();

    try {
        bool success = doc["ok"];

        if (success)
        {
            m_result = doc["token"].get<std::string>().c_str();
            emitSucceeded();
        }
        else
        {
            emitFailed("The API returned an output indicating failure.");
        }
    }
    catch (const nlohmann::json::exception& e)
    {
        qCritical() << "Output: " << res;
        emitFailed("The API returned an unexpected JSON output." + QString::fromStdString(e.what()));
    }
}
