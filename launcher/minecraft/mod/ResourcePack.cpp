#include "ResourcePack.h"

#include <QDebug>
#include <QMap>
#include <QRegularExpression>

#include "Version.h"

#include "minecraft/mod/tasks/LocalResourcePackParseTask.h"

// https://minecraft.wiki/w/Pack_format#List_of_resource_pack_formats
static const QMap<std::pair<int, int>, std::pair<Version, Version>> s_pack_format_versions = {
    { { 1, 0 }, { Version("1.6.1"), Version("1.8.9") } },
    { { 2, 0 }, { Version("1.9"), Version("1.10.2") } },
    { { 3, 0 }, { Version("1.11"), Version("1.12.2") } },
    { { 4, 0 }, { Version("1.13"), Version("1.14.4") } },
    { { 5, 0 }, { Version("1.15"), Version("1.16.1") } },
    { { 6, 0 }, { Version("1.16.2"), Version("1.16.5") } },
    { { 7, 0 }, { Version("1.17"), Version("1.17.1") } },
    { { 8, 0 }, { Version("1.18"), Version("1.18.2") } },
    { { 9, 0 }, { Version("1.19"), Version("1.19.2") } },
    { { 11, 0 }, { Version("22w42a"), Version("22w44a") } },
    { { 12, 0 }, { Version("1.19.3"), Version("1.19.3") } },
    { { 13, 0 }, { Version("1.19.4"), Version("1.19.4") } },
    { { 14, 0 }, { Version("23w14a"), Version("23w16a") } },
    { { 15, 0 }, { Version("1.20"), Version("1.20.1") } },
    { { 16, 0 }, { Version("23w31a"), Version("23w31a") } },
    { { 17, 0 }, { Version("23w32a"), Version("1.20.2-pre1") } },
    { { 18, 0 }, { Version("1.20.2"), Version("1.20.2") } },
    { { 19, 0 }, { Version("23w42a"), Version("23w42a") } },
    { { 20, 0 }, { Version("23w43a"), Version("23w44a") } },
    { { 21, 0 }, { Version("23w45a"), Version("23w46a") } },
    { { 22, 0 }, { Version("1.20.3"), Version("1.20.4") } },
    { { 24, 0 }, { Version("24w03a"), Version("24w04a") } },
    { { 25, 0 }, { Version("24w05a"), Version("24w05b") } },
    { { 26, 0 }, { Version("24w06a"), Version("24w07a") } },
    { { 28, 0 }, { Version("24w09a"), Version("24w10a") } },
    { { 29, 0 }, { Version("24w11a"), Version("24w11a") } },
    { { 30, 0 }, { Version("24w12a"), Version("24w12a") } },
    { { 31, 0 }, { Version("24w13a"), Version("1.20.5-pre3") } },
    { { 32, 0 }, { Version("1.20.5"), Version("1.20.6") } },
    { { 33, 0 }, { Version("24w18a"), Version("24w20a") } },
    { { 34, 0 }, { Version("1.21"), Version("1.21.1") } },
    { { 35, 0 }, { Version("24w33a"), Version("24w33a") } },
    { { 36, 0 }, { Version("24w34a"), Version("24w35a") } },
    { { 37, 0 }, { Version("24w36a"), Version("24w36a") } },
    { { 38, 0 }, { Version("24w37a"), Version("24w37a") } },
    { { 39, 0 }, { Version("24w38a"), Version("24w39a") } },
    { { 40, 0 }, { Version("24w40a"), Version("24w40a") } },
    { { 41, 0 }, { Version("1.21.2-pre1"), Version("1.21.2-pre2") } },
    { { 42, 0 }, { Version("1.21.2"), Version("1.21.3") } },
    { { 43, 0 }, { Version("24w44a"), Version("24w44a") } },
    { { 44, 0 }, { Version("24w45a"), Version("24w45a") } },
    { { 45, 0 }, { Version("24w46a"), Version("24w46a") } },
    { { 46, 0 }, { Version("1.21.4"), Version("1.21.4") } },
    { { 47, 0 }, { Version("25w02a"), Version("25w02a") } },
    { { 48, 0 }, { Version("25w03a"), Version("25w03a") } },
    { { 49, 0 }, { Version("25w04a"), Version("25w04a") } },
    { { 50, 0 }, { Version("25w05a"), Version("25w05a") } },
    { { 51, 0 }, { Version("25w06a"), Version("25w06a") } },
    { { 52, 0 }, { Version("25w07a"), Version("25w07a") } },
    { { 53, 0 }, { Version("25w08a"), Version("25w09b") } },
    { { 54, 0 }, { Version("25w10a"), Version("25w10a") } },
    { { 55, 0 }, { Version("1.21.5"), Version("1.21.5") } },
    { { 56, 0 }, { Version("25w15a"), Version("25w15a") } },
    { { 57, 0 }, { Version("25w16a"), Version("25w16a") } },
    { { 58, 0 }, { Version("25w17a"), Version("25w17a") } },
    { { 59, 0 }, { Version("25w18a"), Version("25w18a") } },
    { { 60, 0 }, { Version("25w19a"), Version("25w19a") } },
    { { 61, 0 }, { Version("25w20a"), Version("25w20a") } },
    { { 62, 0 }, { Version("25w21a"), Version("25w21a") } },
    { { 63, 0 }, { Version("1.21.6"), Version("1.21.6") } },
    { { 64, 0 }, { Version("1.21.7"), Version("1.21.8") } },
    { { 65, 0 }, { Version("25w31a"), Version("25w31a") } },
    { { 65, 1 }, { Version("25w32a"), Version("25w32a") } },
    { { 65, 2 }, { Version("25w33a"), Version("25w33a") } },
    { { 66, 0 }, { Version("25w34a"), Version("25w34b") } },
    { { 67, 0 }, { Version("25w35a"), Version("25w35a") } },
    { { 68, 0 }, { Version("25w36a"), Version("25w36b") } },
    { { 69, 0 }, { Version("1.21.9"), Version("1.21.10") } },
    { { 70, 0 }, { Version("25w41a"), Version("25w41a") } },
    { { 70, 1 }, { Version("25w42a"), Version("25w42a") } },
    { { 71, 0 }, { Version("25w43a"), Version("25w43a") } },
    { { 72, 0 }, { Version("25w44a"), Version("25w44a") } },
    { { 73, 0 }, { Version("25w45a"), Version("25w45a") } },
    { { 74, 0 }, { Version("25w46a"), Version("25w46a") } },
    { { 75, 0 }, { Version("1.21.11"), Version("1.21.11") } },
    { { 76, 0 }, { Version("26.1-snap1"), Version("26.1-snap1") } },
    { { 77, 0 }, { Version("26.1-snap2"), Version("26.1-snap2") } },
    { { 78, 0 }, { Version("26.1-snap3"), Version("26.1-snap3") } },
    { { 78, 1 }, { Version("26.1-snap4"), Version("26.1-snap4") } },
    { { 79, 0 }, { Version("26.1-snap5"), Version("26.1-snap5") } },
    { { 80, 0 }, { Version("26.1-snap6"), Version("26.1-snap6") } },
    { { 81, 0 }, { Version("26.1-snap7"), Version("26.1-snap7") } },
    { { 81, 1 }, { Version("26.1-snap8"), Version("26.1-snap9") } },
    { { 82, 0 }, { Version("26.1-snap10"), Version("26.1-snap10") } },
    { { 83, 0 }, { Version("26.1-snap11"), Version("26.1-snap11") } },
    { { 84, 0 }, { Version("26.1"), Version("26.1.2") } },
    { { 85, 0 }, { Version("26.2-snap1"), Version("26.2-snap2") } },
    { { 86, 0 }, { Version("26.2-snap3"), Version("26.2-snap3") } },
    { { 86, 1 }, { Version("26.2-snap4"), Version("26.2-snap4") } },
    { { 86, 2 }, { Version("26.2-snap5"), Version("26.2-snap6") } },
    { { 87, 0 }, { Version("26.2-snap7"), Version("26.2-snap8") } },
    { { 88, 0 }, { Version("26.2"), Version("26.2") } },
    { { 89, 0 }, { Version("26.3-snap1"), Version("26.3-snap1") } },
    { { 90, 0 }, { Version("26.3-snap2"), Version("26.3-snap2") } },
    { { 91, 0 }, { Version("26.3-snap3"), Version("26.3-snap3") } },
    { { 92, 0 }, { Version("26.3-snap4"), Version("26.3-snap4") } },
    { { 93, 0 }, { Version("26.3-snap5"), Version("26.3-snap5") } },
    { { 94, 0 }, { Version("26.3-snap6"), Version("26.3-snap6") } },
    { { 95, 0 }, { Version("26.3-snap7"), Version("26.3-snap7") } },
    { { 96, 0 }, { Version("26.3-snap8"), Version("26.3-snap9") } },
    { { 97, 0 }, { Version("26.3-snap10"), Version("26.3-snap10") } },
    { { 97, 1 }, { Version("26.3"), Version("26.3") } },
    { { 98, 0 }, { Version("26.4-snap1"), Version("26.4-snap1") } },
    { { 99, 0 }, { Version("26.4-snap2"), Version("26.4-snap2") } },
    { { 100, 0 }, { Version("26.4-snap3"), Version("26.4-snap3") } },
};

void ResourcePack::setPackFormat(int new_format_id, std::pair<int, int> min, std::pair<int, int> max)
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

    if (m_pack_image_cache_key.key.isValid())
        QPixmapCache::remove(m_pack_image_cache_key.key);

    m_pack_image_cache_key.key = QPixmapCache::insert(QPixmap::fromImage(new_image));
    m_pack_image_cache_key.was_ever_used = true;
}

QPixmap ResourcePack::image(QSize size)
{
    QPixmap cached_image;
    if (QPixmapCache::find(m_pack_image_cache_key.key, &cached_image)) {
        if (size.isNull())
            return cached_image;
        return cached_image.scaled(size);
    }

    // No valid image we can get
    if (!m_pack_image_cache_key.was_ever_used)
        return {};

    // Imaged got evicted from the cache. Re-process it and retry.
    ResourcePackUtils::process(*this);
    return image(size);
}

QString ResourcePack::toString() const {
    auto compat = compatibleVersions();
    if (m_pack_format != 0) {
        // this does not have a min/max range
        if (compat.first.toString().isEmpty())
            return QString::number(m_pack_format);

        return QStringLiteral("%1 (%2 - %3)")
            .arg(QString::number(m_pack_format), compat.first.toString(), compat.second.toString());
    }

    // no valid pack format at all
    if (compat.first.toString().isEmpty())
        return tr("Unrecognized");

    // min and max are defined
    auto fmtToStr = [](std::pair<int, int> pair) {
        auto str = QString::number(pair.first);
        if (pair.second != 0)
            str = str % QStringLiteral(".%1").arg(QString::number(pair.second));
        return str;
    };

    const auto minStr = fmtToStr(m_minFormat);
    const auto maxStr = fmtToStr(m_maxFormat);

    auto fmt = minStr;
    if (m_minFormat != m_maxFormat)
        fmt = fmt % QStringLiteral(" - %1").arg(maxStr);

    auto ver = compat.first.toString();
    if (compat.first != compat.second)
        ver = ver % QStringLiteral(" - %1").arg(compat.second.toString());

    return fmt % QStringLiteral(" (%1)")
        .arg(ver);
}

std::pair<Version, Version> ResourcePack::compatibleVersions() const {
    const auto& map = versionMap();

    std::pair<Version, Version> res = { {}, {} };
    if (m_minFormat > m_maxFormat) return res;

    const std::pair<int, int> packFormatPair = { m_pack_format, 0};

    // if a valid min is defined use it as the lower bound
    if (m_minFormat.first != 0 && map.contains(m_minFormat)) {
        res.first = map.constFind(m_minFormat).value().first;

        // if a valid max is defined, use it as the upper bound
        if (m_maxFormat.first != 0 && map.contains(m_maxFormat)) {
            res.second = map.constFind(m_maxFormat).value().second;
        }
        // otherwise assume it supports the newest value in the map
        else {
            res.second = map.last().second;
        }
    }
    // otherwise, use pack format
    else if (m_pack_format != 0 && map.contains(packFormatPair)) {
        res = map.constFind(packFormatPair).value();
    }

    // if no valid version was found it will be unrecognized
    return res;
}

std::pair<int, bool> ResourcePack::compare(const Resource& other, SortType type) const
{
    auto const& cast_other = static_cast<ResourcePack const&>(other);

    switch (type) {
        default: {
            auto res = Resource::compare(other, type);
            if (res.first != 0)
                return res;
            [[fallthrough]];
        }
        case SortType::PACK_FORMAT: {
            auto this_ver = packFormat();
            auto other_ver = cast_other.packFormat();

            if (this_ver > other_ver)
                return { 1, type == SortType::PACK_FORMAT };
            if (this_ver < other_ver)
                return { -1, type == SortType::PACK_FORMAT };
        }
    }
    return { 0, false };
}

bool ResourcePack::applyFilter(QRegularExpression filter) const
{
    if (filter.match(description()).hasMatch())
        return true;

    if (filter.match(QString::number(packFormat())).hasMatch())
        return true;

    if (filter.match(compatibleVersions().first.toString()).hasMatch())
        return true;
    if (filter.match(compatibleVersions().second.toString()).hasMatch())
        return true;

    return Resource::applyFilter(filter);
}

QMap<std::pair<int, int>, std::pair<Version, Version>> ResourcePack::versionMap() const {
    return s_pack_format_versions;
}
