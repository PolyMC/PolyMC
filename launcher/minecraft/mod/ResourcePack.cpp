#include "ResourcePack.h"

#include <QRegularExpression>

#include <iterator>

#include "Version.h"

namespace
{
QString fmtToStr(std::pair<int, int> pair)
{
    auto str = QString::number(pair.first);
    if (pair.second != 0)
    {
        str += QStringLiteral(".%1").arg(QString::number(pair.second));
    }
    return str;
}

const PackFormatVersions* findPackFormat(const PackFormatTable& table,
                                         std::pair<int, int> format)
{
    const PackFormatVersions* found = nullptr;
    qsizetype low = 0;
    qsizetype high = table.m_count;
    while (low < high && found == nullptr)
    {
        const qsizetype middle = low + (high - low) / 2;
        const PackFormatVersions& row = table.m_rows[middle];
        const std::pair<int, int> key = {row.m_major, row.m_minor};
        if (key < format)
        {
            low = middle + 1;
        }
        else if (format < key)
        {
            high = middle;
        }
        else
        {
            found = &row;
        }
    }
    return found;
}
}  // namespace

// https://minecraft.wiki/w/Pack_format#List_of_pack_formats
static constexpr PackFormatVersions s_resource_pack_format_versions[] = {
    {1, 0, "1.6.1", "1.8.9"},
    {2, 0, "1.9", "1.10.2"},
    {3, 0, "1.11", "1.12.2"},
    {4, 0, "1.13", "1.14.4"},
    {5, 0, "1.15", "1.16.1"},
    {6, 0, "1.16.2", "1.16.5"},
    {7, 0, "1.17", "1.17.1"},
    {8, 0, "1.18", "1.18.2"},
    {9, 0, "1.19", "1.19.2"},
    {11, 0, "22w42a", "22w44a"},
    {12, 0, "1.19.3", "1.19.3"},
    {13, 0, "1.19.4", "1.19.4"},
    {14, 0, "23w14a", "23w16a"},
    {15, 0, "1.20", "1.20.1"},
    {16, 0, "23w31a", "23w31a"},
    {17, 0, "23w32a", "1.20.2-pre1"},
    {18, 0, "1.20.2", "1.20.2"},
    {19, 0, "23w42a", "23w42a"},
    {20, 0, "23w43a", "23w44a"},
    {21, 0, "23w45a", "23w46a"},
    {22, 0, "1.20.3", "1.20.4"},
    {24, 0, "24w03a", "24w04a"},
    {25, 0, "24w05a", "24w05b"},
    {26, 0, "24w06a", "24w07a"},
    {28, 0, "24w09a", "24w10a"},
    {29, 0, "24w11a", "24w11a"},
    {30, 0, "24w12a", "24w12a"},
    {31, 0, "24w13a", "1.20.5-pre3"},
    {32, 0, "1.20.5", "1.20.6"},
    {33, 0, "24w18a", "24w20a"},
    {34, 0, "1.21", "1.21.1"},
    {35, 0, "24w33a", "24w33a"},
    {36, 0, "24w34a", "24w35a"},
    {37, 0, "24w36a", "24w36a"},
    {38, 0, "24w37a", "24w37a"},
    {39, 0, "24w38a", "24w39a"},
    {40, 0, "24w40a", "24w40a"},
    {41, 0, "1.21.2-pre1", "1.21.2-pre2"},
    {42, 0, "1.21.2", "1.21.3"},
    {43, 0, "24w44a", "24w44a"},
    {44, 0, "24w45a", "24w45a"},
    {45, 0, "24w46a", "24w46a"},
    {46, 0, "1.21.4", "1.21.4"},
    {47, 0, "25w02a", "25w02a"},
    {48, 0, "25w03a", "25w03a"},
    {49, 0, "25w04a", "25w04a"},
    {50, 0, "25w05a", "25w05a"},
    {51, 0, "25w06a", "25w06a"},
    {52, 0, "25w07a", "25w07a"},
    {53, 0, "25w08a", "25w09b"},
    {54, 0, "25w10a", "25w10a"},
    {55, 0, "1.21.5", "1.21.5"},
    {56, 0, "25w15a", "25w15a"},
    {57, 0, "25w16a", "25w16a"},
    {58, 0, "25w17a", "25w17a"},
    {59, 0, "25w18a", "25w18a"},
    {60, 0, "25w19a", "25w19a"},
    {61, 0, "25w20a", "25w20a"},
    {62, 0, "25w21a", "25w21a"},
    {63, 0, "1.21.6", "1.21.6"},
    {64, 0, "1.21.7", "1.21.8"},
    {65, 0, "25w31a", "25w31a"},
    {65, 1, "25w32a", "25w32a"},
    {65, 2, "25w33a", "25w33a"},
    {66, 0, "25w34a", "25w34b"},
    {67, 0, "25w35a", "25w35a"},
    {68, 0, "25w36a", "25w36b"},
    {69, 0, "1.21.9", "1.21.10"},
    {70, 0, "25w41a", "25w41a"},
    {70, 1, "25w42a", "25w42a"},
    {71, 0, "25w43a", "25w43a"},
    {72, 0, "25w44a", "25w44a"},
    {73, 0, "25w45a", "25w45a"},
    {74, 0, "25w46a", "25w46a"},
    {75, 0, "1.21.11", "1.21.11"},
    {76, 0, "26.1-snap1", "26.1-snap1"},
    {77, 0, "26.1-snap2", "26.1-snap2"},
    {78, 0, "26.1-snap3", "26.1-snap3"},
    {78, 1, "26.1-snap4", "26.1-snap4"},
    {79, 0, "26.1-snap5", "26.1-snap5"},
    {80, 0, "26.1-snap6", "26.1-snap6"},
    {81, 0, "26.1-snap7", "26.1-snap7"},
    {81, 1, "26.1-snap8", "26.1-snap9"},
    {82, 0, "26.1-snap10", "26.1-snap10"},
    {83, 0, "26.1-snap11", "26.1-snap11"},
    {84, 0, "26.1", "26.1.2"},
    {85, 0, "26.2-snap1", "26.2-snap2"},
    {86, 0, "26.2-snap3", "26.2-snap3"},
    {86, 1, "26.2-snap4", "26.2-snap4"},
    {86, 2, "26.2-snap5", "26.2-snap6"},
    {87, 0, "26.2-snap7", "26.2-snap8"},
    {88, 0, "26.2", "26.2"},
    {89, 0, "26.3-snap1", "26.3-snap1"},
    {90, 0, "26.3-snap2", "26.3-snap2"},
    {91, 0, "26.3-snap3", "26.3-snap3"},
    {92, 0, "26.3-snap4", "26.3-snap4"},
    {93, 0, "26.3-snap5", "26.3-snap5"},
    {94, 0, "26.3-snap6", "26.3-snap6"},
    {95, 0, "26.3-snap7", "26.3-snap7"},
    {96, 0, "26.3-snap8", "26.3-snap9"},
    {97, 0, "26.3-snap10", "26.3-snap10"},
    {97, 1, "26.3", "26.3"},
    {98, 0, "26.4-snap1", "26.4-snap1"},
    {99, 0, "26.4-snap2", "26.4-snap2"},
    {100, 0, "26.4-snap3", "26.4-snap3"},
};

static_assert(packFormatsAscend(s_resource_pack_format_versions,
                                std::size(s_resource_pack_format_versions)));

void ResourcePack::setPackFormat(int new_format_id, std::pair<int, int> min,
                                 std::pair<int, int> max)
{
    QMutexLocker locker(&m_data_lock);

    m_pack_format = new_format_id;
    m_minFormat = min;
    m_maxFormat = max;
}

void ResourcePack::setDescription(QString new_description)
{
    QMutexLocker locker(&m_data_lock);

    m_description = new_description;
}

void ResourcePack::setImage(QImage new_image)
{
    QMutexLocker locker(&m_data_lock);

    Q_ASSERT(!new_image.isNull());

    m_pack_image = new_image;
    m_pack_image_cache_key = QPixmapCache::Key();
}

QPixmap ResourcePack::image(QSize size)
{
    QMutexLocker locker(&m_data_lock);

    QPixmap cached_image;
    if (!QPixmapCache::find(m_pack_image_cache_key, &cached_image))
    {
        if (m_pack_image.isNull())
        {
            return {};
        }
        cached_image = QPixmap::fromImage(m_pack_image);
        m_pack_image_cache_key = QPixmapCache::insert(cached_image);
    }

    if (size.isNull())
    {
        return cached_image;
    }
    return cached_image.scaled(size);
}

QString ResourcePack::toString() const
{
    auto compat = compatibleVersions();
    if (m_pack_format != 0)
    {
        // this does not have a min/max range
        if (compat.first.toString().isEmpty())
        {
            return QString::number(m_pack_format);
        }

        return QStringLiteral("%1 (%2 - %3)")
            .arg(QString::number(m_pack_format), compat.first.toString(),
                 compat.second.toString());
    }

    // no valid pack format at all
    if (compat.first.toString().isEmpty())
    {
        return tr("Unrecognized");
    }

    // min and max are defined
    const auto minStr = fmtToStr(m_minFormat);
    const auto maxStr = fmtToStr(m_maxFormat);

    auto fmt = minStr;
    if (m_minFormat != m_maxFormat)
    {
        fmt += QStringLiteral(" - %1").arg(maxStr);
    }

    auto ver = compat.first.toString();
    if (compat.first != compat.second)
    {
        ver += QStringLiteral(" - %1").arg(compat.second.toString());
    }

    return QStringLiteral("%1 (%2)").arg(fmt, ver);
}

std::pair<Version, Version> ResourcePack::compatibleVersions() const
{
    const PackFormatTable table = versionTable();

    std::pair<Version, Version> res = {{}, {}};
    if (m_minFormat > m_maxFormat)
    {
        return res;
    }

    const PackFormatVersions* lower = nullptr;
    const PackFormatVersions* upper = nullptr;

    // if a valid min is defined use it as the lower bound
    if (m_minFormat.first != 0)
    {
        lower = findPackFormat(table, m_minFormat);
    }

    if (lower != nullptr)
    {
        // if a valid max is defined, use it as the upper bound
        if (m_maxFormat.first != 0)
        {
            upper = findPackFormat(table, m_maxFormat);
        }
        // otherwise assume it supports the newest value in the table
        if (upper == nullptr)
        {
            upper = &table.m_rows[table.m_count - 1];
        }
    }
    // otherwise, use pack format
    else if (m_pack_format != 0)
    {
        lower = findPackFormat(table, {m_pack_format, 0});
        upper = lower;
    }

    if (lower != nullptr)
    {
        res.first = Version(QString::fromLatin1(lower->m_first));
        res.second = Version(QString::fromLatin1(upper->m_last));
    }

    // if no valid version was found it will be unrecognized
    return res;
}

std::pair<int, bool> ResourcePack::compare(const Resource& other,
                                           SortType type) const
{
    auto const& cast_other = static_cast<ResourcePack const&>(other);

    switch (type)
    {
        default:
        {
            auto res = Resource::compare(other, type);
            if (res.first != 0)
            {
                return res;
            }
            [[fallthrough]];
        }
        case SortType::PACK_FORMAT:
        {
            auto this_ver = packFormat();
            auto other_ver = cast_other.packFormat();

            if (this_ver > other_ver)
            {
                return {1, type == SortType::PACK_FORMAT};
            }
            if (this_ver < other_ver)
            {
                return {-1, type == SortType::PACK_FORMAT};
            }
        }
    }
    return {0, false};
}

bool ResourcePack::applyFilter(QRegularExpression filter) const
{
    if (filter.match(description()).hasMatch())
    {
        return true;
    }

    if (filter.match(QString::number(packFormat())).hasMatch())
    {
        return true;
    }

    if (filter.match(compatibleVersions().first.toString()).hasMatch())
    {
        return true;
    }
    if (filter.match(compatibleVersions().second.toString()).hasMatch())
    {
        return true;
    }

    return Resource::applyFilter(filter);
}

PackFormatTable ResourcePack::versionTable() const
{
    return {s_resource_pack_format_versions,
            std::size(s_resource_pack_format_versions)};
}

#include "moc_ResourcePack.cpp"
