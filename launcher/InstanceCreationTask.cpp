#include "InstanceCreationTask.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <utility>

#include "modplatform/helpers/ModpackMetadata.h"

InstanceCreationTask::InstanceCreationTask() = default;

void InstanceCreationTask::executeTask()
{
    setAbortable(true);

    if (updateInstance())
    {
        emitSucceeded();
        return;
    }

    // When the user aborted in the update stage.
    if (m_abort)
    {
        emitAborted();
        return;
    }

    if (!createInstance())
    {
        if (m_abort)
        {
            return;
        }

        qWarning() << "Instance creation failed!";
        if (!m_error_message.isEmpty())
        {
            qWarning() << "Reason: " << m_error_message;
        }
        emitFailed(tr("Error while creating new instance."));
        return;
    }

    // If this is set, it means we're updating an instance. So, we now need to
    // remove the files scheduled to, and we'd better not let the user abort in
    // the middle of it, since it'd put the instance in an invalid state.
    if (shouldOverride())
    {
        setAbortable(false);
        setStatus(tr("Removing old conflicting files..."));
        qDebug() << "Removing old files";

        // Removed mods, by mods folder, to remove their metadata too
        QMap<QString, QStringList> removed_mods;
        for (const auto& path : std::as_const(m_files_to_remove))
        {
            QFileInfo file(path);
            if (file.dir().dirName() == "mods")
            {
                auto name = file.fileName();
                if (name.endsWith(".disabled"))
                {
                    name.chop(9);
                }
                removed_mods[file.absolutePath()].append(name);
            }

            if (!QFile::exists(path))
            {
                continue;
            }
            qDebug() << "Removing" << path;
            if (!QFile::remove(path))
            {
                qCritical() << "Couldn't remove the old conflicting files.";
                emitFailed(tr("Failed to remove old conflicting files."));
                return;
            }
        }

        for (auto it = removed_mods.cbegin(); it != removed_mods.cend(); ++it)
        {
            ModpackMetadata::remove(QDir(it.key()), it.value());
        }
    }

    emitSucceeded();
    return;
}

#include "moc_InstanceCreationTask.cpp"
