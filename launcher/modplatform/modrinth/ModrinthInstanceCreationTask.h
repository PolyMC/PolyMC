#pragma once

#include "InstanceCreationTask.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QWeakPointer>
#include <QWidget>

#include <optional>
#include <utility>
#include <vector>

#include "BaseInstance.h"

#include "modplatform/modrinth/ModrinthPackManifest.h"
#include "modplatform/modrinth/ModrinthSource.h"

#include "net/Download.h"
#include "net/NetJob.h"

#include "settings/SettingsObject.h"

class ModrinthCreationTask final : public InstanceCreationTask
{
    Q_OBJECT

public:
    ModrinthCreationTask(QString staging_path,
                         SettingsObjectPtr global_settings, QWidget* parent,
                         QString source_url = {}, Modrinth::Source source = {},
                         QString project = {})
        : InstanceCreationTask(),
          m_parent(parent),
          m_source_url(std::move(source_url)),
          m_pack_source(std::move(source)),
          m_project_id(std::move(project))
    {
        setStagingPath(staging_path);
        setParentSettings(global_settings);
    }

    bool abort() override;

    bool updateInstance() override;
    bool createInstance() override;

private:
    bool parseManifest(const QString&, std::vector<Modrinth::File>&,
                       bool set_managed_info = true,
                       bool show_optional_dialog = true);
    QString getManagedPackID() const;
    // Warns about files not served by cdn.modrinth.com. Returns false if the
    // user declined.
    bool confirmNonModrinthDownloads();
    // The Modrinth-compatible sources the files of the pack may come from:
    // the pack's own source, or for a pack file of unknown origin, every
    // configured one
    QList<Modrinth::Source> fileSources() const;
    // Hosts the pack may also download from: those of its file sources, and
    // wherever a source served the pack from
    QStringList sourceHosts() const;
    bool isSourceDownloadUrl(const QUrl& url) const;
    bool isAllowedDownloadUrl(const QUrl& url) const;
    // The source a mod of the pack is looked up on, by where it is
    // downloaded from. None for files that aren't on a Modrinth source.
    std::optional<Modrinth::Source> metadataSource(const QUrl& url) const;
    void onFileDownloadFailed(Modrinth::File& file, const QString& path,
                              const QWeakPointer<Net::Download>& action);

private:
    QWidget* m_parent = nullptr;

    QString minecraftVersion, fabricVersion, quiltVersion, forgeVersion,
        neoforgeVersion;
    QString m_managed_id, m_managed_version_id, m_managed_name;
    QString m_source_url;
    Modrinth::Source m_pack_source;
    QString m_project_id;

    std::vector<Modrinth::File> m_files;
    NetJob::Ptr m_files_job;

    std::optional<InstancePtr> m_instance;
};
