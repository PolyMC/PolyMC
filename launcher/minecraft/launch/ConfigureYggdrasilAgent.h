// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QByteArray>
#include <QCryptographicHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <memory>

#include <launch/LaunchStep.h>
#include "net/Download.h"
#include "net/HttpMetaCache.h"
#include "net/NetJob.h"

// Gets the Java agent that makes the game use a custom auth server, like
// authlib-injector or Loki, and sets its -javaagent argument. The sources are
// tried in order. When none can be reached, the agent from the last launch
// that got one is used, so launching works offline.
class ConfigureYggdrasilAgent : public LaunchStep
{
    Q_OBJECT
public:
    // server is the auth server's URL, argument receives the -javaagent value
    ConfigureYggdrasilAgent(LaunchTask* parent, QString server,
                            std::shared_ptr<QString> argument);

    void executeTask() override;
    void finalize() override;
    bool canAbort() const override
    {
        return false;
    }

protected:
    struct Source
    {
        // A metadata component, with an index.json and version files listing
        // the jar under +agents, or else authlib-injector's latest.json
        bool component = true;
        // The component's folder, or the latest.json URL
        QUrl url;
        // Keeps the cached files of different sources apart
        QString cachePrefix;
    };

    // The agent's name in messages, and its folder in the cache
    virtual QString agentName() const = 0;
    virtual QString cacheBase() const = 0;
    virtual QList<Source> sources() const = 0;

    // The metadata server set in the settings, empty if there is none
    static QUrl metaUrlOverride();

private:
    void trySource();
    void sourceFailed(QString reason);
    void jobSucceeded();
    void indexDownloaded();
    void versionDownloaded();
    void latestDownloaded();
    void downloadJar(const QUrl& url, QCryptographicHash::Algorithm algorithm,
                     const QByteArray& checksum);
    void jarDownloaded();
    void useLastJar();
    void configure(const QString& path);
    void startDownload(const QString& name, Net::Download::Ptr download,
                       void (ConfigureYggdrasilAgent::*next)());
    QByteArray readEntry();

    QList<Source> m_sources;
    int m_source_index = 0;
    QStringList m_errors;
    std::unique_ptr<NetJob> m_job;
    void (ConfigureYggdrasilAgent::*m_next)() = nullptr;
    MetaEntryPtr m_entry;
    QString m_version;
    std::shared_ptr<QString> m_javaagent_arg;
    QString m_yggdrasil_base_url;
};
