// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
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

#pragma once

#include <QDateTime>
#include <QFutureWatcher>
#include <QTemporaryDir>
#include <memory>

#include "JavaRuntime.h"
#include "net/NetJob.h"
#include "tasks/Task.h"

namespace JavaRuntime
{

/** One downloadable build of Java. */
struct Release
{
    Provider provider = Provider::Mojang;
    int major = 0;
    QString version;
    /** Vendor name of the build, e.g. the release tag or the Mojang component.
     */
    QString name;
    QDateTime released;
    /** "jre" or "jdk". */
    QString type;
    QString url;
    QByteArray sha256;
    QByteArray sha1;
    /** Only set for Mojang runtimes. */
    QString mojangComponent;
    /** A Mojang file manifest, downloaded file by file. Otherwise an archive.
     */
    bool manifest = false;
};

/** Long-term support releases, the ones Minecraft targets. */
bool isLts(int major);

/** Lists the major Java versions a provider offers for this platform, newest
 * first. */
class VersionListTask : public Task
{
    Q_OBJECT
public:
    explicit VersionListTask(Provider provider);

    Provider provider() const
    {
        return m_provider;
    }
    QList<int> majors() const
    {
        return m_majors;
    }

    bool abort() override;

protected:
    void executeTask() override;

private slots:
    void jobSucceeded();
    void jobFailed(QString reason);
    void metaIndexLoaded();
    void metaListLoaded();

private:
    /** Asks the provider itself, when the metadata server doesn't list its
     * runtimes. */
    void fetchFromProvider();
    void parse();

    Provider m_provider;
    NetJob::Ptr m_job;
    Task::Ptr m_metaTask;
    std::shared_ptr<QByteArray> m_data = std::make_shared<QByteArray>();
    QList<int> m_majors;
};

/** Lists the builds a provider offers for one major version on this platform,
 * newest first. */
class ReleaseListTask : public Task
{
    Q_OBJECT
public:
    ReleaseListTask(Provider provider, int major);

    Provider provider() const
    {
        return m_provider;
    }
    int major() const
    {
        return m_major;
    }
    QList<Release> releases() const
    {
        return m_releases;
    }

    /** The build the launcher installs on its own: the newest JRE, or the
     * newest JDK if there is no JRE. */
    std::optional<Release> recommended() const;

    bool abort() override;

protected:
    void executeTask() override;

private slots:
    void jobSucceeded();
    void jobFailed(QString reason);
    void metaIndexLoaded();
    void metaListLoaded();
    void metaVersionLoaded();

private:
    /** Asks the provider itself, when the metadata server doesn't list its
     * runtimes. */
    void fetchFromProvider();
    void parse();

    Provider m_provider;
    int m_major;
    NetJob::Ptr m_job;
    Task::Ptr m_metaTask;
    std::shared_ptr<QByteArray> m_data = std::make_shared<QByteArray>();
    QList<Release> m_releases;
};

/** Downloads and installs a runtime. */
class InstallTask : public Task
{
    Q_OBJECT
public:
    /** Installs exactly this build. */
    explicit InstallTask(Release release);
    /** Installs the recommended build of the first major the provider has. */
    InstallTask(Provider provider, QList<int> majors);

    InstalledRuntime result() const
    {
        return m_result;
    }

    bool abort() override;

protected:
    void executeTask() override;

private slots:
    void jobSucceeded();
    void jobFailed(QString reason);
    void releaseListFinished();
    void extractFinished();

private:
    struct Link
    {
        QString path;
        QString target;
    };

    void resolveNext();
    void download();
    void downloadArchive();
    void extract();
    void downloadMojangFiles();
    void finishMojangFiles();
    void install();

    /** Runs a download job and calls next once it succeeds. */
    void runJob(NetJob::Ptr job, void (InstallTask::*next)());

    Provider m_provider;
    QList<int> m_majors;
    int m_majorIndex = 0;
    QString m_lastError;
    shared_qobject_ptr<ReleaseListTask> m_listTask;

    Release m_release;
    NetJob::Ptr m_job;
    void (InstallTask::*m_afterJob)() = nullptr;
    std::shared_ptr<QByteArray> m_data = std::make_shared<QByteArray>();
    std::unique_ptr<QTemporaryDir> m_tempDir;
    QList<Link> m_links;
    QStringList m_executables;
    QFutureWatcher<QString> m_extractWatcher;
    bool m_aborted = false;

    InstalledRuntime m_result;
};

}  // namespace JavaRuntime
