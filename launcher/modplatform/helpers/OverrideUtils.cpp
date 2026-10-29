#include "OverrideUtils.h"

#include <QDirIterator>

#include "FileSystem.h"

namespace Override {

void createOverrides(const QString& name, const QString& parent_folder, const QString& override_path)
{
    QString file_path(FS::PathCombine(parent_folder, name + ".txt"));
    if (QFile::exists(file_path))
        QFile::remove(file_path);

    FS::ensureFilePathExists(file_path);

    QFile file(file_path);
    if (!file.open(QFile::WriteOnly))
        return;

    QDir override_dir(override_path);
    QDirIterator override_iterator(override_path, QDirIterator::Subdirectories);
    while (override_iterator.hasNext()) {
        auto override_file_path = override_iterator.next();
        QFileInfo info(override_file_path);
        if (info.isFile()) {
            // Absolute path with temp directory -> relative path
            // (relative to the overrides folder itself: splitting at its name breaks when the name appears
            // anywhere else in the path, or when the folder is called something else)
            override_file_path = override_dir.relativeFilePath(override_file_path);

            file.write(override_file_path.toUtf8());
            file.write("\n");
        }
    }

    file.close();
}

QStringList readOverrides(const QString& name, const QString& parent_folder)
{
    QString file_path(FS::PathCombine(parent_folder, name + ".txt"));

    QFile file(file_path);
    if (!file.open(QFile::ReadOnly))
        return {};

    QStringList previous_overrides;

    QString entry;
    do {
        entry = file.readLine();
        previous_overrides.append(entry.trimmed());
    } while (!entry.isEmpty());

    file.close();

    return previous_overrides;
}

}  // namespace Override
