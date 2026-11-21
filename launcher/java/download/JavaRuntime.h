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

#pragma once

#include <QList>
#include <QString>
#include <optional>

/**
 * Java runtimes downloaded and managed by the launcher.
 *
 * Every managed runtime lives in its own folder below runtimesRoot() and carries a
 * java-runtime.json file describing where it came from and where its java binary is.
 */
namespace JavaRuntime {

enum class Provider { Mojang, Adoptium, Azul, Semeru };

QList<Provider> allProviders();
QString providerId(Provider provider);
QString providerName(Provider provider);
std::optional<Provider> providerFromId(const QString& id);

/** Architecture of the host, in the naming used by most vendors: x64, x86, aarch64 or arm. Empty if unsupported. */
QString hostArch();

/** Name of the java executable inside a runtime's bin folder (javaw.exe on Windows, java elsewhere). */
QString javaBinaryName();

struct InstalledRuntime {
    Provider provider = Provider::Mojang;
    int major = 0;
    QString version;
    QString arch;
    /** "jre" or "jdk". */
    QString type;
    /** Absolute path of the runtime folder. */
    QString directory;
    /** Absolute path of the java executable. */
    QString javaPath;

    QString displayName() const;
};

/** Absolute path of the folder holding all managed runtimes. */
QString runtimesRoot();

/** Folder name used for a runtime build. Reinstalling the same build replaces it. */
QString runtimeFolderName(Provider provider, const QString& type, const QString& version, const QString& mojangComponent = QString());

/** Writes the metadata file for a freshly installed runtime. */
bool writeMetadata(const InstalledRuntime& runtime);

/** Scans runtimesRoot() for runtimes with valid metadata and an existing java binary. */
QList<InstalledRuntime> installedRuntimes();

/**
 * Picks an installed runtime for one of the given major versions.
 * Runtimes from the preferred provider win, then the order of the majors list decides.
 */
std::optional<InstalledRuntime> findInstalled(const QList<int>& majors, Provider preferred);

/** Deletes a managed runtime. Refuses to touch anything outside runtimesRoot(). */
bool removeRuntime(const InstalledRuntime& runtime);

}  // namespace JavaRuntime
