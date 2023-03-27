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

#include "UpdateDialog.h"
#include "ui_UpdateDialog.h"
#include <QDebug>
#include <utility>
#include "Application.h"

#include "BuildConfig.h"
#include "HoeDown.h"

UpdateDialog::UpdateDialog(bool hasUpdate, QWidget *parent) : QDialog(parent), ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
    auto channel = APPLICATION->settings()->get("UpdateChannel").toString();
    if(hasUpdate)
    {
        ui->label->setText(tr("A new %1 update is available!").arg(channel));
    }
    else
    {
        ui->label->setText(tr("No %1 updates found. You are running the latest version.").arg(channel));
        ui->btnUpdateNow->setHidden(true);
        ui->btnUpdateLater->setText(tr("Close"));
    }
    ui->changelogBrowser->setHtml(tr("<center><h1>Loading changelog...</h1></center>"));
    loadChangelog();
    restoreGeometry(QByteArray::fromBase64(APPLICATION->settings()->get("UpdateDialogGeometry").toByteArray()));
}

UpdateDialog::~UpdateDialog() = default;

void UpdateDialog::loadChangelog()
{
    auto channel = APPLICATION->settings()->get("UpdateChannel").toString();
    dljob = new NetJob("Changelog", APPLICATION->network());
    QString url;
    if(channel == "stable")
    {
        url = QString("https://raw.githubusercontent.com/PolyMC/PolyMC/%1/changelog.md").arg(channel);
        m_changelogType = CHANGELOG_MARKDOWN;
    }
    else
    {
        url = QString("https://api.github.com/repos/PolyMC/PolyMC/compare/%1...%2").arg(BuildConfig.GIT_COMMIT, channel);
        m_changelogType = CHANGELOG_COMMITS;
    }
    dljob->addNetAction(Net::Download::makeByteArray(QUrl(url), &changelogData));
    connect(dljob.get(), &NetJob::succeeded, this, &UpdateDialog::changelogLoaded);
    connect(dljob.get(), &NetJob::failed, this, &UpdateDialog::changelogFailed);
    dljob->start();
}

QString reprocessMarkdown(QByteArray markdown)
{
    HoeDown hoedown;
    QString output = hoedown.process(std::move(markdown));

    // HACK: easier than customizing hoedown
    output.replace(QRegularExpression("GH-([0-9]+)"), R"(<a href="https://github.com/PolyMC/PolyMC/issues/\1">GH-\1</a>)");
    qDebug() << output;
    return output;
}

QString reprocessCommits(const QByteArray& json)
{
    auto channel = APPLICATION->settings()->get("UpdateChannel").toString();
    try
    {
        QString result;
        nlohmann::json rootobject = nlohmann::json::parse(json.constData(), json.constData() + json.size());

        std::string status = rootobject["status"].get<std::string>();
        QString diff_url = rootobject["html_url"].get<std::string>().c_str();

        auto print_commits = [&]()
        {
            result += "<table cellspacing=0 cellpadding=2 style='border-width: 1px; border-style: solid'>";
            auto commitarray = rootobject["commits"];
            for(int i = commitarray.size() - 1; i >= 0; i--)
            {
                nlohmann::json commitobj = commitarray[i];
                // NOTE: this ignores merge commits, because they have more than one parent
                if(commitobj["parents"].size() > 1)
                {
                    continue;
                }
                QString commit_url = commitobj["html_url"].get<std::string>().c_str();
                QString commit_message = commitobj["commit"]["message"].get<std::string>().c_str();
                auto lines = commit_message.split('\n');
                QRegularExpression regexp("(?<prefix>(GH-(?<issuenr>[0-9]+))|(NOISSUE)|(SCRATCH))? *(?<rest>.*) *");
                auto match = regexp.match(lines.takeFirst(), 0, QRegularExpression::NormalMatch);
                auto issuenr = match.captured("issuenr");
                auto prefix = match.captured("prefix");
                auto rest = match.captured("rest");
                result += "<tr><td>";
                if(issuenr.length())
                {
                    result += QString("<a href=\"https://github.com/PolyMC/PolyMC/issues/%1\">GH-%2</a>").arg(issuenr, issuenr);
                }
                else if(prefix.length())
                {
                    result += QString("<a href=\"%1\">%2</a>").arg(commit_url, prefix);
                }
                else
                {
                    result += QString("<a href=\"%1\">NOISSUE</a>").arg(commit_url);
                }
                result += "</td>";
                lines.prepend(rest);
                result += "<td><p>" + lines.join("<br />") + "</p></td></tr>";
            }
            result += "</table>";
        };

        if(status == "identical")
        {
            return QObject::tr("<p>There are no code changes between your current version and latest %1.</p>").arg(channel);
        }
        else if(status == "ahead")
        {
            result += QObject::tr("<p>Following commits were added since last update:</p>");
            print_commits();
        }
        else if(status == "diverged")
        {
            int commit_ahead = rootobject["ahead_by"].get<int>();
            int commit_behind = rootobject["behind_by"].get<int>();
            result += QObject::tr("<p>The update removes %1 commits and adds the following %2:</p>").arg(commit_behind).arg(commit_ahead);
            print_commits();
        }
        result += QObject::tr("<p>You can <a href=\"%1\">look at the changes on github</a>.</p>").arg(diff_url);
        return result;
    }
    catch (const std::exception& e)
    {
        qWarning() << "Got an unparseable commit log from github:" << e.what();
        qDebug() << json;
    }
    return QString();
}

void UpdateDialog::changelogLoaded()
{
    QString result;
    switch(m_changelogType)
    {
        case CHANGELOG_COMMITS:
            result = reprocessCommits(changelogData);
            break;
        case CHANGELOG_MARKDOWN:
            result = reprocessMarkdown(changelogData);
            break;
    }
    changelogData.clear();
    ui->changelogBrowser->setHtml(result);
}

void UpdateDialog::changelogFailed(const QString& reason)
{
    ui->changelogBrowser->setHtml(tr(R"(<p align="center" <span style="font-size:22pt;">Failed to fetch changelog... Error: %1</span></p>)").arg(reason));
}

void UpdateDialog::on_btnUpdateLater_clicked()
{
    reject();
}

void UpdateDialog::on_btnUpdateNow_clicked()
{
    done(UPDATE_NOW);
}

void UpdateDialog::closeEvent(QCloseEvent* evt)
{
    APPLICATION->settings()->set("UpdateDialogGeometry", saveGeometry().toBase64());
    QDialog::closeEvent(evt);
}
