// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "FileSystem.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>

#include <system_error>

#if defined Q_OS_WIN32
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <sys/utime.h>
#include <windows.h>
#include <winnls.h>
#include <string>
#else
#include <utime.h>
#endif

// Snippet from https://github.com/gulrak/filesystem#using-it-as-single-file-header

#ifdef __APPLE__
#include <Availability.h> // for deployment target to support pre-catalina targets without std::fs
#endif // __APPLE__

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || (defined(__cplusplus) && __cplusplus >= 201703L)) && defined(__has_include)
#if __has_include(<filesystem>) && (!defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500)
#define GHC_USE_STD_FS
#include <filesystem>
namespace fs = std::filesystem;
#endif // MacOS min version check
#endif // Other OSes version check

#ifndef GHC_USE_STD_FS
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#endif

#if defined Q_OS_WIN32

std::wstring toStdString(QString s)
{
    return s.toStdWString();
}

#else

std::string toStdString(QString s)
{
    return s.toStdString();
}

#endif

namespace FS {

void ensureExists(const QDir& dir)
{
    if (!QDir().mkpath(dir.absolutePath())) {
        throw FileSystemException("Unable to create folder " + dir.dirName() + " (" + dir.absolutePath() + ")");
    }
}

void write(const QString& filename, const QByteArray& data)
{
    ensureExists(QFileInfo(filename).dir());
    QSaveFile file(filename);
    if (!file.open(QSaveFile::WriteOnly)) {
        throw FileSystemException("Couldn't open " + filename + " for writing: " + file.errorString());
    }
    if (data.size() != file.write(data)) {
        throw FileSystemException("Error writing data to " + filename + ": " + file.errorString());
    }
    if (!file.commit()) {
        throw FileSystemException("Error while committing data to " + filename + ": " + file.errorString());
    }
}

QByteArray read(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) {
        throw FileSystemException("Unable to open " + filename + " for reading: " + file.errorString());
    }
    const qint64 size = file.size();
    QByteArray data(int(size), 0);
    const qint64 ret = file.read(data.data(), size);
    if (ret == -1 || ret != size) {
        throw FileSystemException("Error reading data from " + filename + ": " + file.errorString());
    }
    return data;
}

bool updateTimestamp(const QString& filename)
{
#ifdef Q_OS_WIN32
    std::wstring filename_utf_16 = filename.toStdWString();
    return (_wutime64(filename_utf_16.c_str(), nullptr) == 0);
#else
    QByteArray filenameBA = QFile::encodeName(filename);
    return (utime(filenameBA.data(), nullptr) == 0);
#endif
}

bool ensureFilePathExists(QString filenamepath)
{
    QFileInfo a(filenamepath);
    QDir dir;
    QString ensuredPath = a.path();
    bool success = dir.mkpath(ensuredPath);
    return success;
}

bool ensureFolderPathExists(const QFileInfo folderPath)
{
    QDir dir;
    QString ensuredPath = folderPath.filePath();
    bool success = dir.mkpath(ensuredPath);
    return success;
}

bool ensureFolderPathExists(const QString folderPathName)
{
    return ensureFolderPathExists(QFileInfo(folderPathName));
}

bool copy::operator()(const QString& offset)
{
    using copy_opts = fs::copy_options;

// NOTE always deep copy on windows. the alternatives are too messy.
#if defined Q_OS_WIN32
    m_followSymlinks = true;
#endif

    auto src = PathCombine(m_src.absolutePath(), offset);
    auto dst = PathCombine(m_dst.absolutePath(), offset);

    std::error_code err{};

    fs::copy_options opt = copy_opts::none;

    // The default behavior is to follow symlinks
    if (!m_followSymlinks)
        opt |= copy_opts::copy_symlinks;

    const auto testAndCopy = [opt, &err](const QString& s, const QString& d) {
        if (ensureFilePathExists(d)) {
            fs::copy(toStdString(s), toStdString(d), opt, err);
        } else {
            // mkpath failed which means the destination directory doesn't exist.
            err = std::make_error_code(std::errc::no_such_file_or_directory);
        }

        if (err) {
            qWarning() << "Failed to copy files:" << QString::fromStdString(err.message());
            qDebug() << "Source file:" << s;
            qDebug() << "Destination file:" << d;
        }
    };

    // We can't use copy_opts::recursive because we need to take into account the
    // blacklisted paths, so we iterate over the source directory, and if there's no blacklist
    // match, we copy the file.
    if (QDir src_dir(src); src_dir.exists()) {
        QDirIterator source_it(src, QDir::Filter::Files | QDir::Filter::Hidden, QDirIterator::Subdirectories);

        while (source_it.hasNext()) {
            auto src_path = source_it.next();
            auto relative_path = src_dir.relativeFilePath(src_path);

            auto dst_path = PathCombine(dst, relative_path);

            if (m_blacklist && m_blacklist->matches(relative_path)) {
                qDebug() << "Attempted to copy blacklisted file:";
                qDebug() << "Source file:" << src_path;
                qDebug() << "Destination file:" << dst_path;
                continue;
            }

            testAndCopy(src_path, dst_path);
        }
    } else { // src_dir could still be a file, try to copy it directly.
        if (m_blacklist && m_blacklist->matches(src)){
            qDebug() << "Attempted to copy blacklisted file:";
            qDebug() << "Source file:" << src;
            qDebug() << "Destination file:" << dst;
        } else {
            testAndCopy(src, dst);
        }
    }

    return err.value() == 0;
}

bool deletePath(QString path)
{
    std::error_code err;

    fs::remove_all(toStdString(path), err);

    if (err) {
        qWarning() << "Failed to remove files:" << QString::fromStdString(err.message());
    }

    return err.value() == 0;
}

bool trash(QString path, QString *pathInTrash = nullptr)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return false;
#else
    return QFile::moveToTrash(path, pathInTrash);
#endif
}

QString PathCombine(const QString& path1, const QString& path2)
{
    if (!path1.size())
        return path2;
    if (!path2.size())
        return path1;
    return QDir::cleanPath(path1 + QDir::separator() + path2);
}

QString PathCombine(const QString& path1, const QString& path2, const QString& path3)
{
    return PathCombine(PathCombine(path1, path2), path3);
}

QString PathCombine(const QString& path1, const QString& path2, const QString& path3, const QString& path4)
{
    return PathCombine(PathCombine(path1, path2, path3), path4);
}

QString AbsolutePath(QString path)
{
    return QFileInfo(path).absolutePath();
}

QString ResolveExecutable(QString path)
{
    if (path.isEmpty()) {
        return QString();
    }
    if (!path.contains('/')) {
        path = QStandardPaths::findExecutable(path);
    }
    QFileInfo pathInfo(path);
    if (!pathInfo.exists() || !pathInfo.isExecutable()) {
        return QString();
    }
    return pathInfo.absoluteFilePath();
}

/**
 * Normalize path
 *
 * Any paths inside the current folder will be normalized to relative paths (to current)
 * Other paths will be made absolute
 */
QString NormalizePath(QString path)
{
    QDir a = QDir::currentPath();
    QString currentAbsolute = a.absolutePath();

    QDir b(path);
    QString newAbsolute = b.absolutePath();

    if (newAbsolute.startsWith(currentAbsolute)) {
        return a.relativeFilePath(newAbsolute);
    } else {
        return newAbsolute;
    }
}

QString badFilenameChars = "\"\\/?<>:;*|!+\r\n";

QString RemoveInvalidFilenameChars(QString string, QChar replaceWith)
{
    for (int i = 0; i < string.length(); i++) {
        if (badFilenameChars.contains(string[i])) {
            string[i] = replaceWith;
        }
    }
    return string;
}

QString DirNameFromString(QString string, QString inDir)
{
    int num = 0;
    QString baseName = RemoveInvalidFilenameChars(string, '-');
    QString dirName;
    do {
        if (num == 0) {
            dirName = baseName;
        } else {
            dirName = baseName + "(" + QString::number(num) + ")";
        }

        // If it's over 9000
        if (num > 9000)
            return "";
        num++;
    } while (QFileInfo(PathCombine(inDir, dirName)).exists());
    return dirName;
}

// Does the folder path contain any '!'? If yes, return true, otherwise false.
// (This is a problem for Java)
bool checkProblemticPathJava(QDir folder)
{
    QString pathfoldername = folder.absolutePath();
    return pathfoldername.contains("!", Qt::CaseInsensitive);
}

QString getDesktopDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
}

// Cross-platform Shortcut creation
bool createShortCut(QString location, QString dest, QStringList args, QString name, QString icon)
{
#if !defined(Q_OS_WIN) && !defined(Q_OS_OSX)
    location = PathCombine(location, name + ".desktop");

    QFile f(location);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&f);

    QString argstring;
    if (!args.empty())
        argstring = " '" + args.join("' '") + "'";

    stream << "[Desktop Entry]"
           << "\n";
    stream << "Type=Application"
           << "\n";
    stream << "TryExec=" << dest.toLocal8Bit() << "\n";
    stream << "Exec=" << dest.toLocal8Bit() << argstring.toLocal8Bit() << "\n";
    stream << "Name=" << name.toLocal8Bit() << "\n";
    stream << "Icon=" << icon.toLocal8Bit() << "\n";

    stream.flush();
    f.close();

    f.setPermissions(f.permissions() | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther);

    return true;
#else
    qWarning("Desktop Shortcuts not supported on your platform!");
    return false;
#endif
}

bool mergeFolders(QString dstpath, QString srcpath)
{
    std::error_code ec;
    fs::path fullSrcPath = srcpath.toStdString();
    fs::path fullDstPath = dstpath.toStdString();
    for (auto& entry : fs::recursive_directory_iterator(fullSrcPath))
    {
        fs::path relativeChild = fs::relative(entry, fullSrcPath);
        if (entry.is_directory())
            if (!fs::exists(fullDstPath / relativeChild))
                fs::create_directory(fullDstPath / relativeChild);
        if (entry.is_regular_file())
        {
            fs::path childDst = fullDstPath / relativeChild;
            if (fs::exists(childDst))
                fs::remove(childDst);
            fs::copy(entry, childDst, fs::copy_options::none, ec);
            if (ec.value() != 0)
                qCritical() << QString("File copy failed with: %1. File was %2 -> %3").arg(QString::fromStdString(ec.message()), entry.path().c_str(), childDst.c_str());
        }
    }

    return ec.value() == 0;
}

}
