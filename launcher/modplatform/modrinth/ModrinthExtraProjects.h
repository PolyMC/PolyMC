// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include "modplatform/modrinth/ModrinthAPI.h"
#include "net/NetJob.h"

// Finds the projects a Modrinth search leaves out: the one a link, ID or
// slug in the search names, which may be unlisted or private, and with a
// token, the user's own projects that aren't public, like drafts
class ModrinthExtraProjects : public QObject
{
    Q_OBJECT

public:
    struct Filter
    {
        QString project_type;
        // Empty for any
        QString minecraft_version;
        QStringList loaders;
    };

    explicit ModrinthExtraProjects(QObject* parent);
    ~ModrinthExtraProjects() override;

    // Stops the previous lookup first
    void search(const ModrinthAPI& api, const QString& term,
                const Filter& filter);
    void stop();

    // A label for projects that aren't public, like "Draft", empty for
    // public ones
    static QString statusLabel(const QJsonObject& project);

signals:
    void projectsFound(QList<QJsonObject> projects);

private:
    void onProjectLoaded();
    void onUserLoaded();
    void onUserProjectsLoaded();

    bool matchesFilter(const QJsonObject& project) const;
    bool matchesTerm(const QJsonObject& project) const;

    ModrinthAPI m_api;
    QString m_term;
    Filter m_filter;

    NetJob::Ptr m_project_job;
    NetJob::Ptr m_user_job;
    NetJob::Ptr m_projects_job;
    QByteArray m_project_response;
    QByteArray m_user_response;
    QByteArray m_projects_response;
};
