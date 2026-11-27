// SPDX-License-Identifier: GPL-3.0-only

#include "ModrinthExtraProjects.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QUrl>

#include "modplatform/modrinth/ModrinthSource.h"

namespace
{
void stopExtraProjectsJob(NetJob::Ptr& job, QObject* receiver)
{
    if (job && job->isRunning())
    {
        job->disconnect(receiver);
        job->abort();
    }
    job.reset();
}

QStringList jsonStrings(const QJsonValue& value)
{
    QStringList strings;
    const auto array = value.toArray();
    for (const auto& item : array)
    {
        strings.append(item.toString());
    }
    return strings;
}
}  // namespace

ModrinthExtraProjects::ModrinthExtraProjects(QObject* parent) : QObject(parent)
{
}

ModrinthExtraProjects::~ModrinthExtraProjects()
{
    stop();
}

void ModrinthExtraProjects::stop()
{
    stopExtraProjectsJob(m_project_job, this);
    stopExtraProjectsJob(m_user_job, this);
    stopExtraProjectsJob(m_projects_job, this);
}

QString ModrinthExtraProjects::statusLabel(const QJsonObject& project)
{
    auto status = project.value("status").toString();
    if (status.isEmpty() || status == "approved" || status == "archived")
    {
        return {};
    }
    if (status == "draft")
    {
        return tr("Draft");
    }
    if (status == "private")
    {
        return tr("Private");
    }
    if (status == "unlisted")
    {
        return tr("Unlisted");
    }
    return status;
}

void ModrinthExtraProjects::search(const ModrinthAPI& api, const QString& term,
                                   const Filter& filter)
{
    stop();

    m_api = api;
    m_term = term.trimmed();
    m_filter = filter;

    auto reference = m_api.projectReference(m_term);
    if (!reference.isEmpty())
    {
        m_project_job = m_api.getProject(reference, &m_project_response);
        connect(m_project_job.get(), &NetJob::succeeded, this,
                &ModrinthExtraProjects::onProjectLoaded);
        m_project_job->start();
    }

    // Without a token there are no projects of the user to find
    if (!Modrinth::tokenFor(QUrl(m_api.apiRoot() + "/user")).isEmpty())
    {
        m_user_job = m_api.getCurrentUser(&m_user_response);
        connect(m_user_job.get(), &NetJob::succeeded, this,
                &ModrinthExtraProjects::onUserLoaded);
        m_user_job->start();
    }
}

bool ModrinthExtraProjects::matchesFilter(const QJsonObject& project) const
{
    if (!m_filter.project_type.isEmpty() &&
        project.value("project_type").toString() != m_filter.project_type)
    {
        return false;
    }
    if (!m_filter.minecraft_version.isEmpty() &&
        !jsonStrings(project.value("game_versions"))
             .contains(m_filter.minecraft_version))
    {
        return false;
    }
    if (m_filter.loaders.isEmpty())
    {
        return true;
    }

    const auto loaders = jsonStrings(project.value("loaders"));
    for (const auto& loader : loaders)
    {
        if (m_filter.loaders.contains(loader))
        {
            return true;
        }
    }
    return false;
}

bool ModrinthExtraProjects::matchesTerm(const QJsonObject& project) const
{
    if (m_term.isEmpty())
    {
        return true;
    }
    for (const auto* field : {"title", "slug", "description", "id"})
    {
        if (project.value(field).toString().contains(m_term,
                                                     Qt::CaseInsensitive))
        {
            return true;
        }
    }
    return false;
}

void ModrinthExtraProjects::onProjectLoaded()
{
    m_project_job.reset();

    // Asked for by name, so only its type has to match: its versions are
    // filtered when picking one. Labrinth only knows the type once a version
    // is processed, until then it's "project".
    auto project = QJsonDocument::fromJson(m_project_response).object();
    auto type = project.value("project_type").toString();
    if (project.isEmpty() ||
        (!m_filter.project_type.isEmpty() && type != m_filter.project_type &&
         type != "project"))
    {
        return;
    }
    emit projectsFound({project});
}

void ModrinthExtraProjects::onUserLoaded()
{
    m_user_job.reset();

    auto user = QJsonDocument::fromJson(m_user_response).object();
    auto id = user.value("id").toString();
    if (id.isEmpty())
    {
        return;
    }

    m_projects_job = m_api.getUserProjects(id, &m_projects_response);
    connect(m_projects_job.get(), &NetJob::succeeded, this,
            &ModrinthExtraProjects::onUserProjectsLoaded);
    m_projects_job->start();
}

void ModrinthExtraProjects::onUserProjectsLoaded()
{
    m_projects_job.reset();

    QList<QJsonObject> found;
    const auto projects = QJsonDocument::fromJson(m_projects_response).array();
    for (const auto& value : projects)
    {
        auto project = value.toObject();
        // Public ones are in the search already
        if (!statusLabel(project).isEmpty() && matchesFilter(project) &&
            matchesTerm(project))
        {
            found.append(project);
        }
    }

    if (!found.isEmpty())
    {
        emit projectsFound(found);
    }
}

#include "moc_ModrinthExtraProjects.cpp"
