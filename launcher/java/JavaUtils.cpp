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

#include <QStringList>
#include <QString>
#include <QDir>
#include <QStringList>

#include <settings/Setting.h>

#include <QDebug>
#include "java/JavaUtils.h"
#include "java/JavaInstallList.h"
#include "FileSystem.h"
#include "Application.h"

#include <set>

#define IBUS "@im=ibus"

JavaUtils::JavaUtils()
{
}

QString stripVariableEntries(QString name, QString target, QString remove)
{
    char delimiter = ':';
#ifdef Q_OS_WIN32
    delimiter = ';';
#endif

    auto targetItems = target.split(delimiter);
    auto toRemove = remove.split(delimiter);

    for (QString item : toRemove) {
        bool removed = targetItems.removeOne(item);
        if (!removed)
            qWarning() << "Entry" << item
                << "could not be stripped from variable" << name;
    }
    return targetItems.join(delimiter);
}

QProcessEnvironment CleanEnviroment()
{
    // prepare the process environment
    QProcessEnvironment rawenv = QProcessEnvironment::systemEnvironment();
    QProcessEnvironment env;

    QStringList ignored =
    {
        "JAVA_ARGS",
        "CLASSPATH",
        "CONFIGPATH",
        "JAVA_HOME",
        "JRE_HOME",
        "_JAVA_OPTIONS",
        "JAVA_OPTIONS",
        "JAVA_TOOL_OPTIONS"
    };

    QStringList stripped =
    {
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD)
        "LD_LIBRARY_PATH",
        "LD_PRELOAD",
#endif
        "QT_PLUGIN_PATH",
        "QT_FONTPATH"
    };
    for(auto key: rawenv.keys())
    {
        auto value = rawenv.value(key);
        // filter out dangerous java crap
        if(ignored.contains(key))
        {
            qDebug() << "Env: ignoring" << key << value;
            continue;
        }

        // These are used to strip the original variables
        // If there is "LD_LIBRARY_PATH" and "LAUNCHER_LD_LIBRARY_PATH", we want to
        // remove all values in "LAUNCHER_LD_LIBRARY_PATH" from "LD_LIBRARY_PATH"
        if(key.startsWith("LAUNCHER_"))
        {
            qDebug() << "Env: ignoring" << key << value;
            continue;
        }
        if(stripped.contains(key))
        {
            QString newValue = stripVariableEntries(key, value, rawenv.value("LAUNCHER_" + key));

            qDebug() << "Env: stripped" << key << value << "to" << newValue;
        }
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD)
        // Strip IBus
        // IBus is a Linux IME framework. For some reason, it breaks MC?
        if (key == "XMODIFIERS" && value.contains(IBUS))
        {
            QString save = value;
            value.replace(IBUS, "");
            qDebug() << "Env: stripped" << IBUS << "from" << save << ":" << value;
        }
#endif
        // qDebug() << "Env: " << key << value;
        env.insert(key, value);
    }
#ifdef Q_OS_LINUX
    // HACK: Workaround for QTBUG-42500
    if(!env.contains("LD_LIBRARY_PATH"))
    {
        env.insert("LD_LIBRARY_PATH", "");
    }
#endif

    return env;
}

JavaInstallPtr JavaUtils::MakeJavaPtr(QString path, QString id, QString arch)
{
    JavaInstallPtr javaVersion(new JavaInstall());

    javaVersion->id = id;
    javaVersion->arch = arch;
    javaVersion->path = path;

    return javaVersion;
}

JavaInstallPtr JavaUtils::GetDefaultJava()
{
    JavaInstallPtr javaVersion(new JavaInstall());

    javaVersion->id = "java";
    javaVersion->arch = "unknown";
#if defined(Q_OS_WIN32)
    javaVersion->path = "javaw";
#else
    javaVersion->path = "java";
#endif

    return javaVersion;
}

QStringList addJavasFromEnv(std::set<QString> javas)
{
    auto env = qEnvironmentVariable("POLYMC_JAVA_PATHS");
#if defined(Q_OS_WIN32)
    QList<QString> javaPaths = env.replace("\\", "/").split(QLatin1String(";"));

    auto envPath = qEnvironmentVariable("PATH");
    QList<QString> javaPathsfromPath = envPath.replace("\\", "/").split(QLatin1String(";"));
    for (QString string : javaPathsfromPath) {
        javaPaths.append(string + "/javaw.exe");
    }
#else
    QList<QString> javaPaths = env.split(QLatin1String(":"));
#endif
    for (QString i : javaPaths) {
        javas.emplace(i);
    };

    QStringList result;
    for (QString i : javas) {
        result.append(i);
    }
    return result;
}

#if defined(Q_OS_WIN32)
QList<JavaInstallPtr> JavaUtils::FindJavaFromRegistryKey(DWORD keyType, QString keyName, QString keyJavaDir, QString subkeySuffix)
{
    QList<JavaInstallPtr> javas;

    QString archType = "unknown";
    if (keyType == KEY_WOW64_64KEY)
        archType = "64";
    else if (keyType == KEY_WOW64_32KEY)
        archType = "32";

    HKEY jreKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyName.toStdWString().c_str(), 0,
                      KEY_READ | keyType | KEY_ENUMERATE_SUB_KEYS, &jreKey) == ERROR_SUCCESS)
    {
        // Read the current type version from the registry.
        // This will be used to find any key that contains the JavaHome value.

        WCHAR subKeyName[255];
        DWORD subKeyNameSize, numSubKeys, retCode;

        // Get the number of subkeys
        RegQueryInfoKeyW(jreKey, NULL, NULL, NULL, &numSubKeys, NULL, NULL, NULL, NULL, NULL,
                        NULL, NULL);

        // Iterate until RegEnumKeyEx fails
        if (numSubKeys > 0)
        {
            for (DWORD i = 0; i < numSubKeys; i++)
            {
                subKeyNameSize = 255;
                retCode = RegEnumKeyExW(jreKey, i, subKeyName, &subKeyNameSize, NULL, NULL, NULL,
                                        NULL);
                QString newSubkeyName = QString::fromWCharArray(subKeyName);
                if (retCode == ERROR_SUCCESS)
                {
                    // Now open the registry key for the version that we just got.
                    QString newKeyName = keyName + "\\" + newSubkeyName + subkeySuffix;

                    HKEY newKey;
                    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, newKeyName.toStdWString().c_str(), 0,
                                      KEY_READ | KEY_WOW64_64KEY, &newKey) == ERROR_SUCCESS)
                    {
                        // Read the JavaHome value to find where Java is installed.
                        DWORD valueSz = 0;
                        if (RegQueryValueExW(newKey, keyJavaDir.toStdWString().c_str(), NULL, NULL, NULL,
                                             &valueSz) == ERROR_SUCCESS)
                        {
                            WCHAR *value = new WCHAR[valueSz];
                            RegQueryValueExW(newKey, keyJavaDir.toStdWString().c_str(), NULL, NULL, (BYTE *)value,
                                             &valueSz);

                            QString newValue = QString::fromWCharArray(value);
                            delete [] value;

                            // Now, we construct the version object and add it to the list.
                            JavaInstallPtr javaVersion(new JavaInstall());

                            javaVersion->id = newSubkeyName;
                            javaVersion->arch = archType;
                            javaVersion->path =
                                QDir(FS::PathCombine(newValue, "bin")).absoluteFilePath("javaw.exe");
                            javas.append(javaVersion);
                        }

                        RegCloseKey(newKey);
                    }
                }
            }
        }

        RegCloseKey(jreKey);
    }

    return javas;
}

QList<QString> JavaUtils::FindJavaPaths()
{
    std::set<QString> javas;

    auto emplaceCandidates = [&javas](QList<JavaInstallPtr> runtimes) {
        for (auto x : runtimes) {
            javas.emplace(x->path);
        }
    };

    // Oracle
    QList<JavaInstallPtr> JRE64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\JavaSoft\\Java Runtime Environment", "JavaHome");
    QList<JavaInstallPtr> JDK64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\JavaSoft\\Java Development Kit", "JavaHome");
    QList<JavaInstallPtr> JRE32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\JavaSoft\\Java Runtime Environment", "JavaHome");
    QList<JavaInstallPtr> JDK32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\JavaSoft\\Java Development Kit", "JavaHome");

    // Oracle for Java 9 and newer
    QList<JavaInstallPtr> NEWJRE64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\JavaSoft\\JRE", "JavaHome");
    QList<JavaInstallPtr> NEWJDK64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\JavaSoft\\JDK", "JavaHome");
    QList<JavaInstallPtr> NEWJRE32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\JavaSoft\\JRE", "JavaHome");
    QList<JavaInstallPtr> NEWJDK32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\JavaSoft\\JDK", "JavaHome");

    // AdoptOpenJDK
    QList<JavaInstallPtr> ADOPTOPENJRE32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\AdoptOpenJDK\\JRE", "Path", "\\hotspot\\MSI");
    QList<JavaInstallPtr> ADOPTOPENJRE64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\AdoptOpenJDK\\JRE", "Path", "\\hotspot\\MSI");
    QList<JavaInstallPtr> ADOPTOPENJDK32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\AdoptOpenJDK\\JDK", "Path", "\\hotspot\\MSI");
    QList<JavaInstallPtr> ADOPTOPENJDK64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\AdoptOpenJDK\\JDK", "Path", "\\hotspot\\MSI");

    // Eclipse Foundation
    QList<JavaInstallPtr> FOUNDATIONJDK32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\Eclipse Foundation\\JDK", "Path", "\\hotspot\\MSI");
    QList<JavaInstallPtr> FOUNDATIONJDK64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\Eclipse Foundation\\JDK", "Path", "\\hotspot\\MSI");

    // Eclipse Adoptium
    QList<JavaInstallPtr> ADOPTIUMJRE32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\Eclipse Adoptium\\JRE", "Path", "\\hotspot\\MSI");
    QList<JavaInstallPtr> ADOPTIUMJRE64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\Eclipse Adoptium\\JRE", "Path", "\\hotspot\\MSI");
    QList<JavaInstallPtr> ADOPTIUMJDK32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\Eclipse Adoptium\\JDK", "Path", "\\hotspot\\MSI");
    QList<JavaInstallPtr> ADOPTIUMJDK64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\Eclipse Adoptium\\JDK", "Path", "\\hotspot\\MSI");

    // Microsoft
    QList<JavaInstallPtr> MICROSOFTJDK64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\Microsoft\\JDK", "Path", "\\hotspot\\MSI");

    // Azul Zulu
    QList<JavaInstallPtr> ZULU64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\Azul Systems\\Zulu", "InstallationPath");
    QList<JavaInstallPtr> ZULU32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\Azul Systems\\Zulu", "InstallationPath");

    // BellSoft Liberica
    QList<JavaInstallPtr> LIBERICA64s = this->FindJavaFromRegistryKey(
        KEY_WOW64_64KEY, "SOFTWARE\\BellSoft\\Liberica", "InstallationPath");
    QList<JavaInstallPtr> LIBERICA32s = this->FindJavaFromRegistryKey(
        KEY_WOW64_32KEY, "SOFTWARE\\BellSoft\\Liberica", "InstallationPath");

    // List x64 before x86
    emplaceCandidates(JRE64s);
    emplaceCandidates(NEWJRE64s);
    emplaceCandidates(ADOPTOPENJRE64s);
    emplaceCandidates(ADOPTIUMJRE64s);
    javas.emplace("C:/Program Files/Java/jre8/bin/javaw.exe");
    javas.emplace("C:/Program Files/Java/jre7/bin/javaw.exe");
    javas.emplace("C:/Program Files/Java/jre6/bin/javaw.exe");
    emplaceCandidates(JDK64s);
    emplaceCandidates(NEWJDK64s);
    emplaceCandidates(ADOPTOPENJDK64s);
    emplaceCandidates(FOUNDATIONJDK64s);
    emplaceCandidates(ADOPTIUMJDK64s);
    emplaceCandidates(MICROSOFTJDK64s);
    emplaceCandidates(ZULU64s);
    emplaceCandidates(LIBERICA64s);

    emplaceCandidates(JRE32s);
    emplaceCandidates(NEWJRE32s);
    emplaceCandidates(ADOPTOPENJRE32s);
    emplaceCandidates(ADOPTIUMJRE32s);
    javas.emplace("C:/Program Files (x86)/Java/jre8/bin/javaw.exe");
    javas.emplace("C:/Program Files (x86)/Java/jre7/bin/javaw.exe");
    javas.emplace("C:/Program Files (x86)/Java/jre6/bin/javaw.exe");
    emplaceCandidates(JDK32s);
    emplaceCandidates(NEWJDK32s);
    emplaceCandidates(ADOPTOPENJDK32s);
    emplaceCandidates(FOUNDATIONJDK32s);
    emplaceCandidates(ADOPTIUMJDK32s);
    emplaceCandidates(ZULU32s);
    emplaceCandidates(LIBERICA32s);

    javas.emplace(this->GetDefaultJava()->path);

    return addJavasFromEnv(javas);
}

#elif defined(Q_OS_MAC)
QList<QString> JavaUtils::FindJavaPaths()
{
    std::set<QString> javas;
    javas.emplace(this->GetDefaultJava()->path);
    javas.emplace("/Applications/Xcode.app/Contents/Applications/Application Loader.app/Contents/MacOS/itms/java/bin/java");
    javas.emplace("/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home/bin/java");
    javas.emplace("/System/Library/Frameworks/JavaVM.framework/Versions/Current/Commands/java");
    QDir libraryJVMDir("/Library/Java/JavaVirtualMachines/");
    QStringList libraryJVMJavas = libraryJVMDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &java, libraryJVMJavas) {
        javas.emplace(libraryJVMDir.absolutePath() + "/" + java + "/Contents/Home/bin/java");
        javas.emplace(libraryJVMDir.absolutePath() + "/" + java + "/Contents/Home/jre/bin/java");
    }
    QDir systemLibraryJVMDir("/System/Library/Java/JavaVirtualMachines/");
    QStringList systemLibraryJVMJavas = systemLibraryJVMDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &java, systemLibraryJVMJavas) {
        javas.emplace(systemLibraryJVMDir.absolutePath() + "/" + java + "/Contents/Home/bin/java");
        javas.emplace(systemLibraryJVMDir.absolutePath() + "/" + java + "/Contents/Commands/java");
    }
    return addJavasFromEnv(javas);
}

#elif defined(Q_OS_LINUX)
QList<QString> JavaUtils::FindJavaPaths()
{
    qDebug() << "Linux Java detection incomplete - defaulting to \"java\"";

    std::set<QString> javas;
    javas.emplace(this->GetDefaultJava()->path);
    auto scanJavaDir = [&](const QString& dirPath) {
        QDir dir(dirPath);
        if (!dir.exists())
            return;
        auto entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (auto& entry : entries) {
            QString prefix = entry.filePath();
            if (entry.isAbsolute()) {
                javas.emplace(QDir{ FS::PathCombine(prefix, "jre/bin/java") }.canonicalPath());
                javas.emplace(QDir{ FS::PathCombine(prefix, "bin/java") }.canonicalPath());
            } else {
                javas.emplace(FS::PathCombine(prefix, "jre/bin/java"));
                javas.emplace(FS::PathCombine(prefix, "bin/java"));
            }
        }
    };
    // oracle RPMs
    scanJavaDir("/usr/java");
    // general locations used by distro packaging
    scanJavaDir("/usr/lib/jvm");
    scanJavaDir("/usr/lib64/jvm");
    scanJavaDir("/usr/lib32/jvm");
    // javas stored in PolyMC's folder
    scanJavaDir("java");
    // manually installed JDKs in /opt
    scanJavaDir("/opt/jdk");
    scanJavaDir("/opt/jdks");
    // flatpak
    scanJavaDir("/app/jdk");

    // Default SDKMAN directory can be overwritten via SDKMAN_DIR env var (default $HOME/.sdkman)
    // see https://sdkman.io/install
    auto sdkmanInstallPath = qEnvironmentVariable("SDKMAN_DIR", FS::PathCombine(QDir::homePath(), ".sdkman"));
    scanJavaDir(FS::PathCombine(sdkmanInstallPath, "candidates/java"));
    // Default ASDF directory can be overwritten via ASDF_DIR or ASDF_DATA_DIR env vars (default $HOME/.asdf)
    // see https://asdf-vm.com/manage/configuration.html#asdf-dir
    auto asdfDataPath = qEnvironmentVariable("ASDF_DATA_DIR", qEnvironmentVariable("ASDF_DIR", FS::PathCombine(QDir::homePath(), ".asdf")));
    scanJavaDir(FS::PathCombine(asdfDataPath, "installs/java"));

    return addJavasFromEnv(javas);
}
#else
QList<QString> JavaUtils::FindJavaPaths()
{
    qDebug() << "Unknown operating system build - defaulting to \"java\"";

    QList<QString> javas;
    javas.append(this->GetDefaultJava()->path);

    return addJavasFromEnv(javas);
}
#endif

QString JavaUtils::getJavaCheckPath()
{
    return APPLICATION->getJarPath("JavaCheck.jar");
}
