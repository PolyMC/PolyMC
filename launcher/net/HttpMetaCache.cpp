// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
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

#include "HttpMetaCache.h"
#include "FileSystem.h"
#include <nlohmann/json.hpp>

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <fstream>

auto MetaEntry::getFullPath() -> QString
{
    // FIXME: make local?
    return FS::PathCombine(m_basePath, m_relativePath);
}

HttpMetaCache::HttpMetaCache(QString path) : QObject(), m_index_file(path)
{
    saveBatchingTimer.setSingleShot(true);
    saveBatchingTimer.setTimerType(Qt::VeryCoarseTimer);

    connect(&saveBatchingTimer, SIGNAL(timeout()), SLOT(SaveNow()));
}

HttpMetaCache::~HttpMetaCache()
{
    saveBatchingTimer.stop();
    SaveNow();
}

auto HttpMetaCache::getEntry(QString base, QString resource_path) -> MetaEntryPtr
{
    // no base. no base path. can't store
    if (!m_entries.contains(base)) {
        // TODO: log problem
        return {};
    }

    EntryMap& map = m_entries[base];
    if (map.entry_list.contains(resource_path)) {
        return map.entry_list[resource_path];
    }

    return {};
}

auto HttpMetaCache::resolveEntry(QString base, QString resource_path, QString expected_etag) -> MetaEntryPtr
{
    auto entry = getEntry(base, resource_path);
    // it's not present? generate a default m_stale entry
    if (!entry) {
        return staleEntry(base, resource_path);
    }

    auto& selected_base = m_entries[base];
    QString real_path = FS::PathCombine(selected_base.base_path, resource_path);
    QFileInfo finfo(real_path);

    // is the file really there? if not -> m_stale
    if (!finfo.isFile() || !finfo.isReadable()) {
        // if the file doesn't exist, we disown the entry
        selected_base.entry_list.remove(resource_path);
        return staleEntry(base, resource_path);
    }

    if (!expected_etag.isEmpty() && expected_etag != entry->m_etag) {
        // if the m_etag doesn't match expected, we disown the entry
        selected_base.entry_list.remove(resource_path);
        return staleEntry(base, resource_path);
    }

    // if the file changed, check m_md5sum
    qint64 file_last_changed = finfo.lastModified().toUTC().toMSecsSinceEpoch();
    if (file_last_changed != entry->m_local_changed_timestamp) {
        QFile input(real_path);
        input.open(QIODevice::ReadOnly);
        QString md5sum = QCryptographicHash::hash(input.readAll(), QCryptographicHash::Md5).toHex().constData();
        if (entry->m_md5sum != md5sum) {
            selected_base.entry_list.remove(resource_path);
            return staleEntry(base, resource_path);
        }

        // md5sums matched... keep entry and save the new state to file
        entry->m_local_changed_timestamp = file_last_changed;
        SaveEventually();
    }

    // Get rid of old entries, to prevent cache problems
    auto current_time = QDateTime::currentSecsSinceEpoch();
    if (entry->isExpired(current_time - ( file_last_changed / 1000 ))) {
        qWarning() << "Removing cache entry because of old age!";
        selected_base.entry_list.remove(resource_path);
        return staleEntry(base, resource_path);
    }

    // entry passed all the checks we cared about.
    entry->m_basePath = getBasePath(base);
    return entry;
}

auto HttpMetaCache::updateEntry(MetaEntryPtr stale_entry) -> bool
{
    if (!m_entries.contains(stale_entry->m_baseId)) {
        qCritical() << "Cannot add entry with unknown base: " << stale_entry->m_baseId.toLocal8Bit();
        return false;
    }

    if (stale_entry->m_stale) {
        qCritical() << "Cannot add m_stale entry: " << stale_entry->getFullPath().toLocal8Bit();
        return false;
    }

    m_entries[stale_entry->m_baseId].entry_list[stale_entry->m_relativePath] = stale_entry;
    SaveEventually();

    return true;
}

auto HttpMetaCache::evictEntry(MetaEntryPtr entry) -> bool
{
    if (!entry)
        return false;

    entry->m_stale = true;
    SaveEventually();
    return true;
}

auto HttpMetaCache::staleEntry(QString base, QString resource_path) -> MetaEntryPtr
{
    auto foo = new MetaEntry();
    foo->m_baseId = base;
    foo->m_basePath = getBasePath(base);
    foo->m_relativePath = resource_path;
    foo->m_stale = true;

    return MetaEntryPtr(foo);
}

void HttpMetaCache::addBase(QString base, QString base_root)
{
    // TODO: report error
    if (m_entries.contains(base))
        return;

    // TODO: check if the base path is valid
    EntryMap foo;
    foo.base_path = base_root;
    m_entries[base] = foo;
}

auto HttpMetaCache::getBasePath(QString base) -> QString
{
    if (m_entries.contains(base)) {
        return m_entries[base].base_path;
    }

    return {};
}

void HttpMetaCache::Load()
{
    if (m_index_file.isNull())
        return;

    nlohmann::json root = nlohmann::json::parse(std::ifstream(m_index_file.toStdString()));

    // check file version first
    if (root["version"].get<std::string>() != "1")
        return;

    // read the entry array
    nlohmann::json temp;
    for (auto& element_obj : root["entries"]) {
        QString base = element_obj["base"].get<std::string>().c_str();
        if (!m_entries.contains(base))
            continue;

        auto& entrymap = m_entries[base];

        auto foo = new MetaEntry();
        foo->m_baseId = base;
        foo->m_relativePath = element_obj.value("path", "").c_str();
        foo->m_md5sum = element_obj.value("m_md5sum", "").c_str();
        foo->m_etag = element_obj.value("m_etag", "").c_str();
        temp = element_obj.value("last_changed_timestamp", nlohmann::json());
        if (!temp.is_null())
            foo->m_local_changed_timestamp = temp.get<qint64>();

        foo->m_remote_changed_timestamp = element_obj.value("m_remote_changed_timestamp", "").c_str();

        foo->makeEternal(element_obj.value("eternal", false));
        if (!foo->isEternal()) {
            temp = element_obj.value("current_age", nlohmann::json());
            if (!temp.is_null())
                foo->m_current_age = temp.get<qint64>();

            temp = element_obj.value("max_age", nlohmann::json());
            if (!temp.is_null())
                foo->m_max_age = temp.get<qint64>();
        }

        // presumed innocent until closer examination
        foo->m_stale = false;

        entrymap.entry_list[foo->m_relativePath] = MetaEntryPtr(foo);
    }
}

void HttpMetaCache::SaveEventually()
{
    // reset the save timer
    saveBatchingTimer.stop();
    saveBatchingTimer.start(30000);
}

void HttpMetaCache::SaveNow()
{
    if (m_index_file.isNull())
        return;

    qDebug() << "[HttpMetaCache]" << "Saving metacache with" << m_entries.size() << "entries";

    nlohmann::json toplevel;
    toplevel["version"] = "1";

    nlohmann::json::array_t entriesArr;
    for (const auto& group : m_entries) {
        for (const auto& entry : group.entry_list) {
            // do not save m_stale entries. they are dead.
            if (entry->m_stale) {
                continue;
            }

            nlohmann::json entryObj;
            entryObj["base"] = entry->m_baseId.toStdString();
            entryObj["path"] = entry->m_relativePath.toStdString();
            entryObj["md5sum"] = entry->m_md5sum.toStdString();
            entryObj["etag"] = entry->m_etag.toStdString();
            entryObj["last_changed_timestamp"] = qint64(entry->m_local_changed_timestamp);

            if (!entry->m_remote_changed_timestamp.isEmpty())
                entryObj["remote_changed_timestamp"] = entry->m_remote_changed_timestamp.toStdString();
            if (entry->isEternal()) {
                entryObj["eternal"] = true;
            } else {
                entryObj["current_age"] = entry->m_current_age;
                entryObj["max_age"] = entry->m_max_age;
            }
            entriesArr.push_back(entryObj);
        }
    }
    toplevel["entries"] = entriesArr;

    try {
        std::ofstream out(m_index_file.toStdString());
        out << toplevel.dump(4);
    } catch (const std::exception& e) {
        qWarning() << e.what();
    }
}
