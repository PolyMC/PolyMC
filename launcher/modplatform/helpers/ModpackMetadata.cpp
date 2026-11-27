// SPDX-License-Identifier: GPL-3.0-only

#include "ModpackMetadata.h"

#include <QDebug>
#include <QEventLoop>
#include <QFileInfo>
#include <QMap>

#include <functional>

#include "minecraft/mod/MetadataHandler.h"
#include "minecraft/mod/Mod.h"
#include "modplatform/EnsureMetadataTask.h"
#include "tasks/SequentialTask.h"

namespace ModpackMetadata
{

namespace
{
void disableModpackModUpdates(QDir index_dir, Mod* mod)
{
    auto metadata = mod->metadata();
    if (!metadata)
    {
        return;
    }

    metadata->do_updates = "false";
    Metadata::update(index_dir, *metadata);
}
}  // namespace

void generate(const QDir& mods, const QList<Entry>& entries)
{
    QDir index_dir(mods.filePath(".index"));

    // One lookup per provider and Modrinth source
    QMap<QString, QList<Entry>> batches;
    for (const auto& entry : entries)
    {
        if (!QFileInfo::exists(mods.filePath(entry.file_name)))
        {
            continue;
        }
        auto key = QString("%1 %2").arg(
            QString::number(static_cast<int>(entry.provider)),
            entry.source.api_url);
        batches[key].append(entry);
    }

    for (const auto& batch : std::as_const(batches))
    {
        QList<Mod*> batch_mods;
        for (const auto& entry : batch)
        {
            batch_mods.append(
                new Mod(QFileInfo(mods.filePath(entry.file_name))));
        }

        auto* task = new EnsureMetadataTask(batch_mods, index_dir,
                                            batch.first().provider,
                                            batch.first().source);
        QObject::connect(task, &EnsureMetadataTask::metadataReady, task,
                         std::bind(&disableModpackModUpdates, index_dir,
                                   std::placeholders::_1));

        SequentialTask sequence(nullptr, "ModpackMetadata");
        if (task->getHashingTask())
        {
            sequence.addTask(task->getHashingTask());
        }
        sequence.addTask(task);

        QEventLoop loop;
        QObject::connect(&sequence, &Task::finished, &loop, &QEventLoop::quit);
        sequence.start();
        if (!sequence.isFinished())
        {
            loop.exec();
        }

        if (!sequence.wasSuccessful())
        {
            qWarning() << "Couldn't look up the mods of the modpack:"
                       << sequence.failReason();
        }

        qDeleteAll(batch_mods);
    }
}

void remove(const QDir& mods, const QStringList& files)
{
    QDir index_dir(mods.filePath(".index"));
    if (files.isEmpty() || !index_dir.exists())
    {
        return;
    }

    const auto index_files =
        index_dir.entryList({"*.pw.toml"}, QDir::Files | QDir::Hidden);
    for (const auto& index_file : index_files)
    {
        auto metadata = Metadata::get(index_dir, index_file);
        if (metadata.isValid() && files.contains(metadata.filename))
        {
            qDebug() << "Removing the metadata of" << metadata.filename;
            Metadata::remove(index_dir, index_file);
        }
    }
}

}  // namespace ModpackMetadata
