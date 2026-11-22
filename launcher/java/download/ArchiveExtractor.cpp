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

#include "ArchiveExtractor.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>

#include <zlib.h>

#include "MMCZip.h"

namespace ArchiveExtractor
{

namespace
{

// Reads a gzip stream as plain bytes, a chunk at a time.
class GzReader
{
public:
    explicit GzReader(QFile& file) : m_file(file)
    {
        m_ok = inflateInit2(&m_stream, 16 + MAX_WBITS) == Z_OK;
    }
    ~GzReader()
    {
        inflateEnd(&m_stream);
    }

    bool ok() const
    {
        return m_ok;
    }

    // Reads exactly `size` bytes. Returns false on error or a premature end of
    // the stream.
    bool read(char* out, qint64 size)
    {
        while (size > 0)
        {
            if (m_outPos == m_outEnd && !fill())
            {
                return false;
            }
            qint64 n = qMin<qint64>(size, m_outEnd - m_outPos);
            memcpy(out, m_out + m_outPos, n);
            m_outPos += n;
            out += n;
            size -= n;
        }
        return true;
    }

    bool skip(qint64 size)
    {
        char buffer[8192];
        while (size > 0)
        {
            qint64 n = qMin<qint64>(size, sizeof(buffer));
            if (!read(buffer, n))
            {
                return false;
            }
            size -= n;
        }
        return true;
    }

private:
    bool fill()
    {
        if (!m_ok)
        {
            return false;
        }
        m_outPos = 0;
        m_outEnd = 0;
        while (m_outEnd == 0)
        {
            if (m_stream.avail_in == 0)
            {
                qint64 got = m_file.read(m_in, BUFFER_SIZE);
                if (got <= 0)
                {
                    return false;
                }
                m_stream.next_in = reinterpret_cast<Bytef*>(m_in);
                m_stream.avail_in = static_cast<uInt>(got);
            }
            m_stream.next_out = reinterpret_cast<Bytef*>(m_out);
            m_stream.avail_out = BUFFER_SIZE;
            int ret = inflate(&m_stream, Z_NO_FLUSH);
            if (ret == Z_STREAM_END)
            {
                // concatenated gzip members are valid gzip
                inflateReset(&m_stream);
            }
            else if (ret != Z_OK)
            {
                m_ok = false;
                return false;
            }
            m_outEnd = BUFFER_SIZE - m_stream.avail_out;
        }
        return true;
    }

    QFile& m_file;
    z_stream m_stream{};
    bool m_ok = false;
    static constexpr int BUFFER_SIZE = 65536;
    QByteArray m_inBuffer = QByteArray(BUFFER_SIZE, '\0');
    QByteArray m_outBuffer = QByteArray(BUFFER_SIZE, '\0');
    char* m_in = m_inBuffer.data();
    char* m_out = m_outBuffer.data();
    qint64 m_outPos = 0;
    qint64 m_outEnd = 0;
};

QString field(const char* data, int size)
{
    int len = 0;
    while (len < size && data[len] != '\0')
    {
        len++;
    }
    return QString::fromUtf8(data, len);
}

qint64 octal(const char* data, int size)
{
    // GNU base-256 encoding for large values
    if (static_cast<unsigned char>(data[0]) & 0x80)
    {
        qint64 value = 0;
        for (int i = 1; i < size; i++)
        {
            value = (value << 8) | static_cast<unsigned char>(data[i]);
        }
        return value;
    }
    qint64 value = 0;
    for (int i = 0; i < size && data[i]; i++)
    {
        if (data[i] == ' ')
        {
            continue;
        }
        if (data[i] < '0' || data[i] > '7')
        {
            break;
        }
        value = value * 8 + (data[i] - '0');
    }
    return value;
}

// Parses "<len> key=value\n" records of a pax extended header.
void parsePax(const QByteArray& data, QString& path, QString& linkPath)
{
    int pos = 0;
    while (pos < data.size())
    {
        int space = data.indexOf(' ', pos);
        if (space < 0)
        {
            return;
        }
        bool ok = false;
        int length = data.mid(pos, space - pos).toInt(&ok);
        if (!ok || length <= 0 || pos + length > data.size())
        {
            return;
        }
        auto record = data.mid(space + 1, length - (space - pos) - 2);
        int eq = record.indexOf('=');
        if (eq > 0)
        {
            auto key = record.left(eq);
            auto value = QString::fromUtf8(record.mid(eq + 1));
            if (key == "path")
            {
                path = value;
            }
            else if (key == "linkpath")
            {
                linkPath = value;
            }
        }
        pos += length;
    }
}

// Maps an archive entry name to a path below root, or returns an empty string
// if it would escape.
QString safePath(const QDir& root, const QString& name)
{
    if (name.isEmpty() || name.startsWith('/') || name.contains('\\') ||
        QDir::isAbsolutePath(name))
    {
        return {};
    }
#ifdef Q_OS_WIN
    // drive-relative paths ("C:foo") and NTFS alternate data streams
    // ("file:stream")
    if (name.contains(':'))
        return {};
#endif
    auto path = QDir::cleanPath(root.absoluteFilePath(name));
    auto rootPath = QDir::cleanPath(root.absolutePath());
    if (path != rootPath && !path.startsWith(rootPath + '/'))
    {
        return {};
    }
    return path;
}

QFileDevice::Permissions toPermissions(qint64 mode)
{
    QFileDevice::Permissions perms;
    if (mode & 0400)
    {
        perms |= QFileDevice::ReadOwner | QFileDevice::ReadUser;
    }
    if (mode & 0200)
    {
        perms |= QFileDevice::WriteOwner | QFileDevice::WriteUser;
    }
    if (mode & 0100)
    {
        perms |= QFileDevice::ExeOwner | QFileDevice::ExeUser;
    }
    if (mode & 0040)
    {
        perms |= QFileDevice::ReadGroup;
    }
    if (mode & 0020)
    {
        perms |= QFileDevice::WriteGroup;
    }
    if (mode & 0010)
    {
        perms |= QFileDevice::ExeGroup;
    }
    if (mode & 0004)
    {
        perms |= QFileDevice::ReadOther;
    }
    if (mode & 0002)
    {
        perms |= QFileDevice::WriteOther;
    }
    if (mode & 0001)
    {
        perms |= QFileDevice::ExeOther;
    }
    return perms;
}

QString extractTarGz(const QString& archive, const QString& target)
{
    QFile file(archive);
    if (!file.open(QIODevice::ReadOnly))
    {
        return QObject::tr("Could not open %1: %2")
            .arg(archive, file.errorString());
    }

    GzReader reader(file);
    if (!reader.ok())
    {
        return QObject::tr("Could not initialize the decompressor.");
    }

    QDir root(target);
    if (!root.mkpath("."))
    {
        return QObject::tr("Could not create %1").arg(target);
    }

    QString longName;
    QString longLink;
    QString paxPath;
    QString paxLink;
    char header[512];

    struct Link
    {
        QString path;
        QString target;
        bool hard;
    };
    QList<Link> links;

    while (true)
    {
        if (!reader.read(header, sizeof(header)))
        {
            return QObject::tr("Unexpected end of archive.");
        }

        // two zero blocks end the archive, one is enough to stop
        bool empty = true;
        for (char c : header)
        {
            if (c != 0)
            {
                empty = false;
                break;
            }
        }
        if (empty)
        {
            break;
        }

        qint64 size = octal(header + 124, 12);
        qint64 mode = octal(header + 100, 8);
        char type = header[156];
        qint64 padded = (size + 511) & ~qint64(511);

        if (type == 'L' || type == 'K' || type == 'x')
        {
            if (size > 1024 * 1024)
            {
                return QObject::tr("Archive contains an oversized header.");
            }
            QByteArray data(padded, '\0');
            if (!reader.read(data.data(), padded))
            {
                return QObject::tr("Unexpected end of archive.");
            }
            data.truncate(size);
            if (type == 'L')
            {
                longName = field(data.constData(), data.size());
            }
            else if (type == 'K')
            {
                longLink = field(data.constData(), data.size());
            }
            else
            {
                parsePax(data, paxPath, paxLink);
            }
            continue;
        }

        QString name = field(header, 100);
        if (memcmp(header + 257, "ustar", 5) == 0)
        {
            auto prefix = field(header + 345, 155);
            if (!prefix.isEmpty())
            {
                name = prefix + '/' + name;
            }
        }
        QString linkName = field(header + 157, 100);
        if (!longName.isEmpty())
        {
            name = longName;
        }
        if (!paxPath.isEmpty())
        {
            name = paxPath;
        }
        if (!longLink.isEmpty())
        {
            linkName = longLink;
        }
        if (!paxLink.isEmpty())
        {
            linkName = paxLink;
        }
        longName.clear();
        longLink.clear();
        paxPath.clear();
        paxLink.clear();

        while (name.startsWith("./"))
        {
            name = name.mid(2);
        }
        if (name.isEmpty() || name == ".")
        {
            if (!reader.skip(padded))
            {
                return QObject::tr("Unexpected end of archive.");
            }
            continue;
        }

        auto path = safePath(root, name);
        if (path.isEmpty())
        {
            return QObject::tr(
                       "Archive entry %1 points outside of the target folder.")
                .arg(name);
        }

        switch (type)
        {
            case '0':
            case '\0':
            case '7':
            {
                QDir().mkpath(QFileInfo(path).absolutePath());
                QFile out(path);
                if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
                {
                    return QObject::tr("Could not write %1: %2")
                        .arg(path, out.errorString());
                }
                QByteArray buffer(65536, '\0');
                qint64 left = size;
                while (left > 0)
                {
                    qint64 n = qMin<qint64>(left, buffer.size());
                    if (!reader.read(buffer.data(), n))
                    {
                        return QObject::tr("Unexpected end of archive.");
                    }
                    if (out.write(buffer.constData(), n) != n)
                    {
                        return QObject::tr("Could not write %1: %2")
                            .arg(path, out.errorString());
                    }
                    left -= n;
                }
                out.close();
                if (mode)
                {
                    out.setPermissions(toPermissions(mode));
                }
                if (!reader.skip(padded - size))
                {
                    return QObject::tr("Unexpected end of archive.");
                }
                break;
            }
            case '5':
                if (!QDir().mkpath(path))
                {
                    return QObject::tr("Could not create %1").arg(path);
                }
                if (!reader.skip(padded))
                {
                    return QObject::tr("Unexpected end of archive.");
                }
                break;
            case '1':
            case '2':
            {
                // symlink targets are relative to the link, hard link targets
                // to the archive root
                QString resolved =
                    type == '2'
                        ? QFileInfo(path).absoluteDir().absoluteFilePath(
                              linkName)
                        : root.absoluteFilePath(linkName);
                resolved = QDir::cleanPath(resolved);
                if (linkName.startsWith('/') ||
                    safePath(root, root.relativeFilePath(resolved)).isEmpty())
                {
                    return QObject::tr(
                               "Archive link %1 points outside of the target "
                               "folder.")
                        .arg(name);
                }
                links.append(
                    {path, type == '2' ? linkName : resolved, type == '1'});
                if (!reader.skip(padded))
                {
                    return QObject::tr("Unexpected end of archive.");
                }
                break;
            }
            default:
                // devices, fifos, global pax headers and the like are not
                // needed for a JRE
                if (!reader.skip(padded))
                {
                    return QObject::tr("Unexpected end of archive.");
                }
                break;
        }
    }

    // links go last, so their targets exist
    for (auto& link : links)
    {
        QDir().mkpath(QFileInfo(link.path).absolutePath());
        QFile::remove(link.path);
        bool ok = link.hard ? QFile::copy(link.target, link.path)
                            : QFile::link(link.target, link.path);
        if (!ok)
        {
            qWarning() << "Could not create link" << link.path << "->"
                       << link.target;
        }
    }
    return {};
}

}  // namespace

QString extract(const QString& archive, const QString& target)
{
    QFile file(archive);
    if (!file.open(QIODevice::ReadOnly))
    {
        return QObject::tr("Could not open %1: %2")
            .arg(archive, file.errorString());
    }
    auto magic = file.read(4);
    file.close();

    if (magic.startsWith("\x1f\x8b"))
    {
        return extractTarGz(archive, target);
    }

    if (magic.startsWith("PK"))
    {
        if (!MMCZip::extractDir(archive, target))
        {
            return QObject::tr("Could not extract %1").arg(archive);
        }
        return {};
    }
    return QObject::tr("Unknown archive format: %1").arg(archive);
}

}  // namespace ArchiveExtractor
