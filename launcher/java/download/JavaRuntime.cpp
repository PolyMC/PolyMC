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

#include "JavaRuntime.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSysInfo>

#include <algorithm>

#include "FileSystem.h"
#include "Version.h"

namespace JavaRuntime
{

static const QString METADATA_FILE = QStringLiteral("java-runtime.json");

QList<Provider> allProviders()
{
    return {Provider::Mojang, Provider::Adoptium, Provider::Azul,
            Provider::Semeru};
}

QString providerId(Provider provider)
{
    switch (provider)
    {
        case Provider::Mojang:
            return "mojang";
        case Provider::Adoptium:
            return "adoptium";
        case Provider::Azul:
            return "azul";
        case Provider::Semeru:
            return "semeru";
    }
    return {};
}

QString providerName(Provider provider)
{
    switch (provider)
    {
        case Provider::Mojang:
            return QObject::tr("Mojang");
        case Provider::Adoptium:
            return QObject::tr("Adoptium");
        case Provider::Azul:
            return QObject::tr("Azul Zulu");
        case Provider::Semeru:
            return QObject::tr("IBM Semeru");
    }
    return {};
}

QString metaUid(Provider provider)
{
    switch (provider)
    {
        case Provider::Mojang:
            return "net.minecraft.java";
        case Provider::Adoptium:
            return "net.adoptium.java";
        case Provider::Azul:
            return "com.azul.java";
        case Provider::Semeru:
            return "com.ibm.java";
    }
    return {};
}

std::optional<Provider> providerFromId(const QString& id)
{
    for (auto provider : allProviders())
    {
        if (providerId(provider) == id)
        {
            return provider;
        }
    }
    return std::nullopt;
}

QString hostArch()
{
    auto arch = QSysInfo::currentCpuArchitecture();
    if (arch == "x86_64")
    {
        return "x64";
    }
    if (arch == "i386")
    {
        return "x86";
    }
    if (arch == "arm64")
    {
        return "aarch64";
    }
    if (arch == "arm")
    {
        return "arm";
    }
    return {};
}

QString javaBinaryName()
{
#ifdef Q_OS_WIN
    return "javaw.exe";
#else
    return "java";
#endif
}

QString InstalledRuntime::displayName() const
{
    if (type.isEmpty())
    {
        return QString("%1 %2").arg(providerName(provider), version);
    }
    return QString("%1 %2 (%3)")
        .arg(providerName(provider), version, type.toUpper());
}

QString runtimesRoot()
{
    return QDir::current().absoluteFilePath("java");
}

QString runtimeFolderName(Provider provider, const QString& type,
                          const QString& version,
                          const QString& mojangComponent)
{
    // Mojang only ever has one build per component, newer builds replace it
    if (provider == Provider::Mojang && !mojangComponent.isEmpty())
    {
        return FS::RemoveInvalidFilenameChars(
            QString("mojang-%1").arg(mojangComponent));
    }
    auto name = QString("%1-%2-%3").arg(providerId(provider), type, version);
    name.replace(QRegularExpression("[^A-Za-z0-9._-]"), "_");
    return name;
}

bool writeMetadata(const InstalledRuntime& runtime)
{
    QDir dir(runtime.directory);
    QJsonObject obj;
    obj.insert("formatVersion", 1);
    obj.insert("provider", providerId(runtime.provider));
    obj.insert("major", runtime.major);
    obj.insert("version", runtime.version);
    obj.insert("arch", runtime.arch);
    obj.insert("type", runtime.type);
    obj.insert("javaPath", dir.relativeFilePath(runtime.javaPath));
    try
    {
        FS::write(dir.absoluteFilePath(METADATA_FILE),
                  QJsonDocument(obj).toJson());
    }
    catch (const Exception& e)
    {
        qWarning() << "Could not write Java runtime metadata:" << e.cause();
        return false;
    }
    return true;
}

static std::optional<InstalledRuntime> readMetadata(const QString& directory)
{
    QDir dir(directory);
    QFile file(dir.absoluteFilePath(METADATA_FILE));
    if (!file.open(QIODevice::ReadOnly))
    {
        return std::nullopt;
    }

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "Invalid Java runtime metadata in" << directory
                   << error.errorString();
        return std::nullopt;
    }
    auto obj = doc.object();

    auto provider = providerFromId(obj.value("provider").toString());
    if (!provider)
    {
        return std::nullopt;
    }

    InstalledRuntime runtime;
    runtime.provider = *provider;
    runtime.major = obj.value("major").toInt();
    runtime.version = obj.value("version").toString();
    runtime.arch = obj.value("arch").toString();
    runtime.type = obj.value("type").toString();
    runtime.directory = dir.absolutePath();
    runtime.javaPath =
        QDir::cleanPath(dir.absoluteFilePath(obj.value("javaPath").toString()));

    // the metadata must not point outside of its own runtime
    if (!runtime.javaPath.startsWith(runtime.directory + '/'))
    {
        return std::nullopt;
    }
    if (!QFileInfo(runtime.javaPath).isFile())
    {
        return std::nullopt;
    }
    return runtime;
}

static bool runtimeOrder(const InstalledRuntime& a, const InstalledRuntime& b)
{
    if (a.major != b.major)
    {
        return a.major > b.major;
    }
    if (a.provider != b.provider)
    {
        return a.provider < b.provider;
    }
    // newest build first, so lookups pick it
    return Version(b.version) < Version(a.version);
}

static bool runsOnHost(const InstalledRuntime& runtime, const QString& host)
{
    return runtime.arch.isEmpty() || runtime.arch == host;
}

QList<InstalledRuntime> installedRuntimes()
{
    QList<InstalledRuntime> result;
    QDir root(runtimesRoot());
    if (!root.exists())
    {
        return result;
    }

    for (auto& entry :
         root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        if (auto runtime = readMetadata(entry.absoluteFilePath()))
        {
            result.append(*runtime);
        }
    }
    std::sort(result.begin(), result.end(), runtimeOrder);
    return result;
}

std::optional<InstalledRuntime> findInstalled(const QList<int>& majors,
                                              Provider preferred)
{
    auto installed = installedRuntimes();
    auto host = hostArch();

    for (int major : majors)
    {
        for (auto& runtime : installed)
        {
            if (runtime.major == major && runtime.provider == preferred &&
                runsOnHost(runtime, host))
            {
                return runtime;
            }
        }
    }
    for (int major : majors)
    {
        for (auto& runtime : installed)
        {
            if (runtime.major == major && runsOnHost(runtime, host))
            {
                return runtime;
            }
        }
    }
    return std::nullopt;
}

bool removeRuntime(const InstalledRuntime& runtime)
{
    auto root = QDir::cleanPath(runtimesRoot());
    auto dir = QDir::cleanPath(runtime.directory);
    if (!dir.startsWith(root + '/') ||
        QFileInfo(dir).absoluteDir().absolutePath() != root)
    {
        qWarning() << "Refusing to delete Java runtime outside of" << root
                   << ":" << dir;
        return false;
    }
    return FS::deletePath(dir);
}

}  // namespace JavaRuntime
