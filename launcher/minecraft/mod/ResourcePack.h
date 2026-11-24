#pragma once

#include "Resource.h"

#include <QImage>
#include <QMutex>
#include <QPixmap>
#include <QPixmapCache>
#include <QSize>
#include <QString>

#include <utility>

class Version;

struct PackFormatVersions
{
    int m_major;
    int m_minor;
    const char* m_first;
    const char* m_last;
};

struct PackFormatTable
{
    const PackFormatVersions* m_rows;
    qsizetype m_count;
};

constexpr bool packFormatsAscend(const PackFormatVersions* rows,
                                 qsizetype count)
{
    bool ascending = true;
    for (qsizetype i = 1; i < count && ascending; ++i)
    {
        const PackFormatVersions& previous = rows[i - 1];
        const PackFormatVersions& current = rows[i];
        ascending = previous.m_major < current.m_major ||
                    (previous.m_major == current.m_major &&
                     previous.m_minor < current.m_minor);
    }
    return ascending;
}

/* TODO:
 *
 * Store localized descriptions
 * */

class ResourcePack : public Resource
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<Resource>;

    ResourcePack(QObject* parent = nullptr) : Resource(parent)
    {
    }
    ResourcePack(QFileInfo file_info) : Resource(file_info)
    {
    }

    /** Gets the numerical ID of the pack format. */
    [[nodiscard]] int packFormat() const
    {
        return m_pack_format;
    }

    /** Get a human-readable display of the pack format */
    QString toString() const;

    /** Gets, respectively, the lower and upper versions supported by the set
     * pack format. */
    [[nodiscard]] std::pair<Version, Version> compatibleVersions() const;

    /** Gets the description of the resource pack. */
    [[nodiscard]] QString description() const
    {
        return m_description;
    }

    /** Gets the image of the resource pack, converted to a QPixmap for drawing,
     * and scaled to size.
     */
    [[nodiscard]] QPixmap image(QSize size);

    /** Thread-safe. */
    void setPackFormat(int new_format_id, std::pair<int, int> min,
                       std::pair<int, int> max);

    /** Thread-safe. */
    void setDescription(QString new_description);

    /** Thread-safe. */
    void setImage(QImage new_image);

    [[nodiscard]] std::pair<int, bool> compare(Resource const& other,
                                               SortType type) const override;
    [[nodiscard]] bool applyFilter(QRegularExpression filter) const override;

protected:
    mutable QMutex m_data_lock;

    virtual PackFormatTable versionTable() const;

    /* The 'version' of a resource pack, as defined in the pack.mcmeta file.
     * See
     * https://minecraft.wiki/w/Tutorials/Creating_a_resource_pack#Formatting_pack.mcmeta
     */
    int m_pack_format = 0;

    // {major, minor}
    std::pair<int, int> m_minFormat;
    std::pair<int, int> m_maxFormat;

    /** The resource pack's description, as defined in the pack.mcmeta file.
     */
    QString m_description;

    QImage m_pack_image;
    QPixmapCache::Key m_pack_image_cache_key;
};
