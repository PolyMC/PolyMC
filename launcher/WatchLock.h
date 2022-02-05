
#pragma once

#include <QString>
#include <QFileSystemWatcher>
#include <utility>

struct WatchLock
{
    WatchLock(QFileSystemWatcher * watcher, QString directory)
        : m_watcher(watcher), m_directory(std::move(directory))
    {
        m_watcher->removePath(m_directory);
    }
    ~WatchLock()
    {
        m_watcher->addPath(m_directory);
    }
    QFileSystemWatcher * m_watcher;
    QString m_directory;
};
