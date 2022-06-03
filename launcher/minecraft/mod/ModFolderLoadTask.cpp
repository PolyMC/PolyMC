#include "ModFolderLoadTask.h"

#include <QDebug>
#include <QMimeDatabase>

ModFolderLoadTask::ModFolderLoadTask(QDir dir) : m_dir(dir), m_result(new Result()) {}

void ModFolderLoadTask::run()
{
    m_dir.refresh();

    QMimeDatabase db;
    for (auto entry : m_dir.entryInfoList()) {
        QMimeType type = db.mimeTypeForFile(entry.fileName());
        if (type.name() == "text/plain")
            continue;

        Mod m(entry);
        m_result->mods[m.mmc_id()] = m;
    }

    emit succeeded();
}
