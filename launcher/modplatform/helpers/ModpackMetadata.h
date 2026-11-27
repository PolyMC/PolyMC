// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDir>
#include <QList>
#include <QString>
#include <QStringList>

#include "modplatform/ModIndex.h"
#include "modplatform/modrinth/ModrinthSource.h"

namespace ModpackMetadata
{

// A mod a modpack installed, and where to look it up
struct Entry
{
    // In the mods folder, with .disabled if the pack left it disabled
    QString file_name;
    ModPlatform::Provider provider;
    // Only used with Modrinth
    Modrinth::Source source;
};

// Looks up the mods a modpack installed into the mods folder and writes their
// metadata with update checks disabled. They follow the pack: updated on their
// own, an old copy would be left behind when the pack updates them. Mods that
// can't be found get no metadata.
void generate(const QDir& mods, const QList<Entry>& entries);

// Removes the metadata of mod files a modpack update removed from the mods
// folder
void remove(const QDir& mods, const QStringList& files);

}  // namespace ModpackMetadata
