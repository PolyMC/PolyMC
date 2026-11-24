// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2026 crueter <crueter@crueter.xyz>
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
 */

#include "DataPack.h"

#include <iterator>

// Values taken from:
// https://minecraft.wiki/w/Pack_format#List_of_pack_formats
static constexpr PackFormatVersions s_data_pack_format_versions[] = {
    {4, 0, "1.13", "1.14.4"},
    {5, 0, "1.15", "1.16.1"},
    {6, 0, "1.16.2", "1.16.5"},
    {7, 0, "1.17", "1.17.1"},
    {8, 0, "1.18", "1.18.1"},
    {9, 0, "1.18.2", "1.18.2"},
    {10, 0, "1.19", "1.19.3"},
    {11, 0, "23w03a", "23w05a"},
    {12, 0, "1.19.4", "1.19.4"},
    {13, 0, "23w12a", "23w14a"},
    {14, 0, "23w16a", "23w17a"},
    {15, 0, "1.20", "1.20.1"},
    {16, 0, "23w31a", "23w31a"},
    {17, 0, "23w32a", "23w35a"},
    {18, 0, "1.20.2", "1.20.2"},
    {19, 0, "23w40a", "23w40a"},
    {20, 0, "23w41a", "23w41a"},
    {21, 0, "23w42a", "23w42a"},
    {22, 0, "23w43a", "23w43b"},
    {23, 0, "23w44a", "23w44a"},
    {24, 0, "23w45a", "23w45a"},
    {25, 0, "23w46a", "23w46a"},
    {26, 0, "1.20.3", "1.20.4"},
    {27, 0, "23w51a", "23w51b"},
    {28, 0, "24w03a", "24w03b"},
    {29, 0, "24w04a", "24w04a"},
    {30, 0, "24w05a", "24w05b"},
    {31, 0, "24w06a", "24w06a"},
    {32, 0, "24w07a", "24w07a"},
    {33, 0, "24w09a", "24w09a"},
    {34, 0, "24w10a", "24w10a"},
    {35, 0, "24w11a", "24w11a"},
    {36, 0, "24w12a", "24w12a"},
    {37, 0, "24w13a", "24w13a"},
    {38, 0, "24w14a", "24w14a"},
    {39, 0, "1.20.5-pre1", "1.20.5-pre1"},
    {40, 0, "1.20.5-pre2", "1.20.5-pre2"},
    {41, 0, "1.20.5", "1.20.6"},
    {42, 0, "24w18a", "24w18a"},
    {43, 0, "24w19a", "24w19b"},
    {44, 0, "24w20a", "24w20a"},
    {45, 0, "24w21a", "24w21b"},
    {46, 0, "1.21-pre1", "1.21-pre1"},
    {47, 0, "1.21-pre2", "1.21-pre2"},
    {48, 0, "1.21", "1.21.1"},
    {49, 0, "24w33a", "24w33a"},
    {50, 0, "24w34a", "24w34a"},
    {51, 0, "24w35a", "24w35a"},
    {52, 0, "24w36a", "24w36a"},
    {53, 0, "24w37a", "24w37a"},
    {54, 0, "24w38a", "24w38a"},
    {55, 0, "24w39a", "24w39a"},
    {56, 0, "24w40a", "24w40a"},
    {57, 0, "1.21.2", "1.21.3"},
    {58, 0, "24w44a", "24w44a"},
    {59, 0, "24w45a", "24w45a"},
    {60, 0, "24w46a", "1.21.4-pre1"},
    {61, 0, "1.21.4", "1.21.4"},
    {62, 0, "25w02a", "25w02a"},
    {63, 0, "25w03a", "25w03a"},
    {64, 0, "25w04a", "25w04a"},
    {65, 0, "25w05a", "25w05a"},
    {66, 0, "25w06a", "25w06a"},
    {67, 0, "25w07a", "25w07a"},
    {68, 0, "25w08a", "25w08a"},
    {69, 0, "25w09a", "25w09b"},
    {70, 0, "25w10a", "1.21.5-pre1"},
    {71, 0, "1.21.5", "1.21.5"},
    {72, 0, "25w15a", "25w15a"},
    {73, 0, "25w16a", "25w16a"},
    {74, 0, "25w17a", "25w17a"},
    {75, 0, "25w18a", "25w18a"},
    {76, 0, "25w19a", "25w19a"},
    {77, 0, "25w20a", "25w20a"},
    {78, 0, "25w21a", "25w21a"},
    {79, 0, "1.21.6-pre1", "1.21.6-pre2"},
    {80, 0, "1.21.6", "1.21.6"},
    {81, 0, "1.21.7", "1.21.8"},
    {82, 0, "25w31a", "25w31a"},
    {83, 0, "25w32a", "25w32a"},
    {83, 1, "25w33a", "25w33a"},
    {84, 0, "25w34a", "25w34b"},
    {85, 0, "25w35a", "25w35a"},
    {86, 0, "25w36a", "25w36b"},
    {87, 0, "25w37a", "25w37a"},
    {87, 1, "1.21.9-pre1", "1.21.9-pre1"},
    {88, 0, "1.21.9", "1.21.10"},
    {89, 0, "25w41a", "25w41a"},
    {90, 0, "25w42a", "25w42a"},
    {91, 0, "25w43a", "25w43a"},
    {92, 0, "25w44a", "25w44a"},
    {93, 0, "25w45a", "25w45a"},
    {93, 1, "25w46a", "25w46a"},
    {94, 0, "1.21.11-pre1", "1.21.11-pre3"},
    {94, 1, "1.21.11-pre4", "1.21.11"},
    {95, 0, "26.1-snap1", "26.1-snap1"},
    {96, 0, "26.1-snap2", "26.1-snap2"},
    {97, 0, "26.1-snap3", "26.1-snap3"},
    {97, 1, "26.1-snap4", "26.1-snap4"},
    {98, 0, "26.1-snap5", "26.1-snap5"},
    {99, 0, "26.1-snap6", "26.1-snap6"},
    {99, 1, "26.1-snap7", "26.1-snap7"},
    {99, 2, "26.1-snap8", "26.1-snap9"},
    {99, 3, "26.1-snap10", "26.1-snap10"},
    {100, 0, "26.1-snap11", "26.1-snap11"},
    {101, 0, "26.1-pre1", "26.1-pre2"},
    {101, 1, "26.1", "26.1.2"},
    {101, 2, "26.2-snap1", "26.2-snap2"},
    {102, 0, "26.2-snap3", "26.2-snap3"},
    {103, 0, "26.2-snap4", "26.2-snap4"},
    {104, 0, "26.2-snap5", "26.2-snap5"},
    {105, 0, "26.2-snap6", "26.2-snap6"},
    {105, 1, "26.2-snap7", "26.2-snap7"},
    {106, 0, "26.2-snap8", "26.2-snap8"},
    {106, 1, "26.2-pre1", "26.2-pre1"},
    {107, 0, "26.2-pre2", "26.2-pre3"},
    {107, 1, "26.2", "26.2"},
    {108, 0, "26.3-snap1", "26.3-snap1"},
    {109, 0, "26.3-snap2", "26.3-snap2"},
    {110, 0, "26.3-snap3", "26.3-snap3"},
    {111, 0, "26.3-snap4", "26.3-snap4"},
    {112, 0, "26.3-snap5", "26.3-snap5"},
    {113, 0, "26.3-snap6", "26.3-snap6"},
    {115, 0, "26.3-snap7", "26.3-snap7"},
    {116, 0, "26.3-snap8", "26.3-snap8"},
    {117, 0, "26.3-snap9", "26.3-snap9"},
    {118, 0, "26.3-snap10", "26.3-snap10"},
    {119, 0, "26.3-pre1", "26.3-pre1"},
    {120, 0, "26.3-pre2", "26.3-pre2"},
    {121, 0, "26.3", "26.3"},
    {122, 0, "26.4-snap1", "26.4-snap1"},
    {122, 1, "26.4-snap2", "26.4-snap2"},
    {123, 0, "26.4-snap3", "26.4-snap3"},
};

static_assert(packFormatsAscend(s_data_pack_format_versions,
                                std::size(s_data_pack_format_versions)));

PackFormatTable DataPack::versionTable() const
{
    return {s_data_pack_format_versions,
            std::size(s_data_pack_format_versions)};
}
