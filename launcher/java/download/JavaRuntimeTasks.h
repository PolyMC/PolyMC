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
#include <functional>
#include <memory>

#include "JavaRuntime.h"
#include "net/NetJob.h"
#include "tasks/Task.h"

namespace JavaRuntime {

/** One downloadable build of Java. */
struct Release {
    Provider provider = Provider::Mojang;
    int major = 0;
    QString version;
    /** Vendor name of the build, e.g. the release tag or the Mojang component. */
    QString name;
    QDateTime released;
    /** "jre" or "jdk". */
    QString type;
    QString url;
    QByteArray sha256;
    QByteArray sha1;
    /** Only set for Mojang runtimes. */
    QString mojangComponent;
};

/** Long-term support releases, the ones Minecraft targets. */
bool isLts(int major);

/** Lists the major Java versions a provider offers for this platform, newest first. */
class VersionListTask : public Task {
    Q_OBJECT
   public:
    explicit VersionListTask(Provider provider);

    QList<int> majors() const { return m_majors; }

    bool abort() override;

   protected:
    void executeTask() override;

   private:
    void parse();

    Provider m_provider;
    NetJob::Ptr m_job;
    std::shared_ptr<QByteArray> m_data = std::make_shared<QByteArray>();
    QList<int> m_majors;
};

/** Lists the builds a provider offers for one major version on this platform, newest first. */
class ReleaseListTask : public Task {
    Q_OBJECT
   public:
    ReleaseListTask(Provider provider, int major);

    QList<Release> releases() const { return m_releases; }

    /** The build the launcher installs on its own: the newest JRE, or the newest JDK if there is no JRE. */
    std::optional<Release> recommended() const;

    bool abort() override;

   protected:
    void executeTask() override;

   private:
    void parse();

    Provider m_provider;
    int m_major;
    NetJob::Ptr m_job;
    std::shared_ptr<QByteArray> m_data = std::make_shared<QByteArray>();
    QList<Release> m_releases;
};

/** Downloads and installs a runtime. */
class InstallTask : public Task {
    Q_OBJECT
   public:
    /** Installs exactly this build. */
    explicit InstallTask(Release release);
    /** Installs the recommended build of the first major the provider has. */
    InstallTask(Provider provider, QList<int> majors);

    InstalledRuntime result() const { return m_result; }

    bool abort() override;

   protected:
    void executeTask() override;

   private:
    void resolveNext();
    void download();
    void downloadArchive();
    void extractFinished();
    void downloadMojangFiles();
    void install();

    void runJob(NetJob::Ptr job, std::function<void()> onSuccess);

    Provider m_provider;
    QList<int> m_majors;
    int m_majorIndex = 0;
    QString m_lastError;
    shared_qobject_ptr<ReleaseListTask> m_listTask;

    Release m_release;
    NetJob::Ptr m_job;
    std::shared_ptr<QByteArray> m_data = std::make_shared<QByteArray>();
    std::unique_ptr<QTemporaryDir> m_tempDir;
    QFutureWatcher<QString> m_extractWatcher;
    bool m_aborted = false;

    InstalledRuntime m_result;
};

}  // namespace JavaRuntime
