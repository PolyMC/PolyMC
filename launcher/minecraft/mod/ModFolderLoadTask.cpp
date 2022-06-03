#include "ModFolderLoadTask.h"
#include <QDebug>

ModFolderLoadTask::ModFolderLoadTask(QDir dir) :
    m_dir(dir), m_result(new Result())
{
}

void ModFolderLoadTask::run()
{
    m_dir.refresh();
    for (auto entry : m_dir.entryInfoList())
    {
        if (entry.fileName().endsWith("txt")) continue;

        Mod m(entry);
        m_result->mods[m.mmc_id()] = m;
    }
    emit succeeded();
}
