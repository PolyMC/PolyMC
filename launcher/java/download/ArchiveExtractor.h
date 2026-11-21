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

#include <QString>

namespace ArchiveExtractor {

/**
 * Extracts a .tar.gz or .zip archive into the target folder.
 * Entries that would land outside of the target (absolute paths, "..", escaping symlinks) are rejected.
 * Unix permissions and symlinks are kept for tar archives.
 *
 * Returns an empty string on success, an error message otherwise.
 */
QString extract(const QString& archive, const QString& target);

}  // namespace ArchiveExtractor
