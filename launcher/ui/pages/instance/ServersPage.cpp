// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
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

#include "ServersPage.h"
#include "ui_ServersPage.h"

#include <FileSystem.h>
#include <sstream>
#include <io/stream_reader.h>
#include <tag_string.h>
#include <tag_primitive.h>
#include <tag_list.h>
#include <tag_compound.h>
#include <minecraft/MinecraftInstance.h>

#include <QFileSystemWatcher>
#include <QMenu>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QDateTime>
#include <QDnsLookup>
#include <QHostAddress>

static const int COLUMN_COUNT = 4; // Name, Address, Players, Ping

struct Server
{
    // Types
    enum class AcceptsTextures : int
    {
        ASK = 0,
        ALWAYS = 1,
        NEVER = 2
    };

    // Methods
    Server()
    {
        m_name = QObject::tr("Minecraft Server");
    }
    Server(const QString & name, const QString & address)
    {
        m_name = name;
        m_address = address;
    }
    Server(nbt::tag_compound& server)
    {
        std::string addressStr(server["ip"]);
        m_address = QString::fromUtf8(addressStr.c_str());

        std::string nameStr(server["name"]);
        m_name = QString::fromUtf8(nameStr.c_str());

        if(server["icon"])
        {
            std::string base64str(server["icon"]);
            m_icon = QByteArray::fromBase64(base64str.c_str());
        }

        if(server.has_key("acceptTextures", nbt::tag_type::Byte))
        {
            bool value = server["acceptTextures"].as<nbt::tag_byte>().get();
            if(value)
            {
                m_acceptsTextures = AcceptsTextures::ALWAYS;
            }
            else
            {
                m_acceptsTextures = AcceptsTextures::NEVER;
            }
        }
    }

    void serialize(nbt::tag_compound& server)
    {
        server.insert("name", m_name.trimmed().toUtf8().toStdString());
        server.insert("ip", m_address.trimmed().toUtf8().toStdString());
        if(m_icon.size())
        {
            server.insert("icon", m_icon.toBase64().toStdString());
        }
        if(m_acceptsTextures != AcceptsTextures::ASK)
        {
            server.insert("acceptTextures", nbt::tag_byte(m_acceptsTextures == AcceptsTextures::ALWAYS));
        }
    }

    // Data - persistent and user changeable
    QString m_name;
    QString m_address;
    AcceptsTextures m_acceptsTextures = AcceptsTextures::ASK;

    // Data - persistent and automatically updated
    QByteArray m_icon;

    // Data - temporary
    bool m_checked = false;
    bool m_up = false;
    QString m_motd; // https://mctools.org/motd-creator
    int m_ping = 0;
    int m_currentPlayers = 0;
    int m_maxPlayers = 0;
};

static std::unique_ptr <nbt::tag_compound> parseServersDat(const QString& filename)
{
    try
    {
        QByteArray input = FS::read(filename);
        std::istringstream foo(std::string(input.constData(), input.size()));
        auto pair = nbt::io::read_compound(foo);

        if(pair.first != "")
            return nullptr;

        if(pair.second == nullptr)
            return nullptr;

        return std::move(pair.second);
    }
    catch (...)
    {
        return nullptr;
    }
}

static bool serializeServerDat(const QString& filename, nbt::tag_compound * levelInfo)
{
    try
    {
        if(!FS::ensureFilePathExists(filename))
        {
            return false;
        }
        std::ostringstream s;
        nbt::io::write_tag("", *levelInfo, s);
        QByteArray val(s.str().data(), (int) s.str().size() );
        FS::write(filename, val);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

static QString stripMinecraftFormatting(const QString& str)
{
    QString result;
    int i = 0;
    while (i < str.size()) {
        if (str[i] == QChar(0xA7) && i + 1 < str.size()) {
            i += 2;
        } else {
            result.append(str[i]);
            ++i;
        }
    }
    return result;
}

class ServerPinger : public QObject
{
    Q_OBJECT
public:
    explicit ServerPinger(const QString& host, quint16 port, QObject* parent = nullptr)
        : QObject(parent), m_host(host), m_port(port)
    {
        m_socket = new QTcpSocket(this);
        connect(m_socket, &QTcpSocket::connected, this, &ServerPinger::onConnected);
        connect(m_socket, &QTcpSocket::readyRead,  this, &ServerPinger::onReadyRead);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        connect(m_socket, &QAbstractSocket::errorOccurred, this, &ServerPinger::onSocketError);
#else
        connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
                this, &ServerPinger::onSocketError);
#endif
        m_timeout.setSingleShot(true);
        m_timeout.setInterval(5000);
        connect(&m_timeout, &QTimer::timeout, this, &ServerPinger::onTimeout);

        m_dns = new QDnsLookup(QDnsLookup::SRV, "_minecraft._tcp." + host, this);
        connect(m_dns, &QDnsLookup::finished, this, &ServerPinger::onSrvLookupDone);
    }

    void ping()
    {
        m_timeout.start();
        // Skip SRV lookup for bare IP addresses
        if (QHostAddress().setAddress(m_host)) {
            m_socket->connectToHost(m_host, m_port);
        } else {
            m_dns->lookup();
        }
    }

signals:
    void done(bool success, int currentPlayers, int maxPlayers, int pingMs, const QString& motd, const QByteArray& icon);

private slots:
    void onConnected()
    {
        // Handshake packet
        QByteArray handshake;
        writeVarInt(handshake, 0x00);
        writeVarInt(handshake, 47);      // Protocol version (1.8; servers accept any for status)
        writeString(handshake, m_host);
        handshake.append(static_cast<char>((m_port >> 8) & 0xFF)); // Port high byte
        handshake.append(static_cast<char>( m_port        & 0xFF)); // Port low byte
        writeVarInt(handshake, 1);       // Next state: status (1)
        m_socket->write(framePacket(handshake));

        // Status Request packet
        QByteArray statusReq;
        writeVarInt(statusReq, 0x00);
        m_socket->write(framePacket(statusReq));

        m_state = WaitingStatusResponse;
    }

    void onReadyRead()
    {
        m_buffer.append(m_socket->readAll());
        tryParsePackets();
    }

    void onSocketError(QAbstractSocket::SocketError)
    {
        m_timeout.stop();
        emit done(false, 0, 0, 0, {}, {});
        deleteLater();
    }

    void onTimeout()
    {
        m_socket->abort();
        emit done(false, 0, 0, 0, {}, {});
        deleteLater();
    }

    void onSrvLookupDone()
    {
        if (m_dns->error() == QDnsLookup::NoError && !m_dns->serviceRecords().isEmpty()) {
            auto srv = m_dns->serviceRecords().first();
            m_host = srv.target();
            if (m_host.endsWith('.'))
                m_host.chop(1);
            m_port = srv.port();
        }
        m_socket->connectToHost(m_host, m_port);
    }

private:
    enum State { Idle, WaitingStatusResponse, WaitingPong };

    static void writeVarInt(QByteArray& buf, int value)
    {
        do {
            quint8 temp = static_cast<quint8>(value & 0x7F);
            value = static_cast<int>(static_cast<unsigned int>(value) >> 7);
            if (value != 0)
                temp |= 0x80;
            buf.append(static_cast<char>(temp));
        } while (value != 0);
    }

    static void writeString(QByteArray& buf, const QString& str)
    {
        QByteArray utf8 = str.toUtf8();
        writeVarInt(buf, utf8.size());
        buf.append(utf8);
    }

    static QByteArray framePacket(const QByteArray& payload)
    {
        QByteArray result;
        writeVarInt(result, payload.size());
        result.append(payload);
        return result;
    }

    bool tryReadVarInt(int& pos, int& out) const
    {
        out = 0;
        int shift = 0;
        while (true) {
            if (pos >= m_buffer.size())
                return false;
            quint8 b = static_cast<quint8>(m_buffer.at(pos++));
            out |= (b & 0x7F) << shift;
            shift += 7;
            if (!(b & 0x80))
                return true;
            if (shift >= 35)
                return false;
        }
    }

    void tryParsePackets()
    {
        while (!m_buffer.isEmpty()) {
            int pos = 0;
            int packetLen;
            if (!tryReadVarInt(pos, packetLen) || packetLen <= 0)
                return;
            // Wait until the full packet body is buffered
            if (m_buffer.size() - pos < packetLen)
                return;

            int packetEnd = pos + packetLen;
            int packetId;
            if (!tryReadVarInt(pos, packetId)) {
                m_buffer.remove(0, packetEnd);
                continue;
            }

            if (m_state == WaitingStatusResponse && packetId == 0x00) {
                // Status Response: VarInt length-prefixed JSON string
                int jsonLen;
                if (!tryReadVarInt(pos, jsonLen) || pos + jsonLen > packetEnd) {
                    m_buffer.remove(0, packetEnd);
                    continue;
                }
                QString jsonStr = QString::fromUtf8(m_buffer.constData() + pos, jsonLen);
                QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
                QJsonObject root = doc.object();

                QJsonValue desc = root["description"];
                if (desc.isObject())
                    m_motd = desc.toObject()["text"].toString();
                else if (desc.isString())
                    m_motd = desc.toString();

                QJsonObject players = root["players"].toObject();
                m_currentPlayers = players["online"].toInt();
                m_maxPlayers     = players["max"].toInt();

                QString favicon = root["favicon"].toString();
                if (favicon.startsWith("data:image/png;base64,"))
                    m_icon = QByteArray::fromBase64(favicon.mid(22).toLatin1());

                QByteArray pingPayload;
                writeVarInt(pingPayload, 0x01);
                qint64 ts = QDateTime::currentMSecsSinceEpoch();
                for (int i = 7; i >= 0; i--)
                    pingPayload.append(static_cast<char>((ts >> (i * 8)) & 0xFF));
                m_socket->write(framePacket(pingPayload));
                m_pingTimer.start();
                m_state = WaitingPong;

            } else if (m_state == WaitingPong && packetId == 0x01) {
                m_timeout.stop();
                int pingMs = static_cast<int>(m_pingTimer.elapsed());
                emit done(true, m_currentPlayers, m_maxPlayers, pingMs, m_motd, m_icon);
                m_socket->close();
                deleteLater();
                return;
            }

            m_buffer.remove(0, packetEnd);
        }
    }

    QTcpSocket*   m_socket;
    QDnsLookup*   m_dns;
    QTimer        m_timeout;
    QElapsedTimer m_pingTimer;
    QString       m_host;
    quint16       m_port;
    QByteArray    m_buffer;
    State         m_state = Idle;
    QString       m_motd;
    QByteArray    m_icon;
    int           m_currentPlayers = 0;
    int           m_maxPlayers     = 0;
};

class ServersModel: public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        ServerPtrRole = Qt::UserRole,
    };
    explicit ServersModel(const QString &path, QObject *parent = 0)
        : QAbstractListModel(parent)
    {
        m_path = path;
        m_watcher = new QFileSystemWatcher(this);
        connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &ServersModel::fileChanged);
        connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &ServersModel::dirChanged);
        m_saveTimer.setSingleShot(true);
        m_saveTimer.setInterval(5000);
        connect(&m_saveTimer, &QTimer::timeout, this, &ServersModel::save_internal);
    }
    virtual ~ServersModel() {};

    void observe()
    {
        if(m_observed)
        {
            return;
        }
        m_observed = true;

        if(!m_loaded)
        {
            load();
        }

        updateFSObserver();
    }

    void unobserve()
    {
        if(!m_observed)
        {
            return;
        }
        m_observed = false;

        updateFSObserver();
    }

    void lock()
    {
        if(m_locked)
        {
            return;
        }
        saveNow();

        m_locked = true;
        updateFSObserver();
    }

    void unlock()
    {
        if(!m_locked)
        {
            return;
        }
        m_locked = false;

        updateFSObserver();
    }

    int addEmptyRow(int position)
    {
        if(m_locked)
        {
            return -1;
        }
        if(position < 0 || position >= rowCount())
        {
            position = rowCount();
        }
        beginInsertRows(QModelIndex(), position, position);
        m_servers.insert(position, Server());
        endInsertRows();
        scheduleSave();
        return position;
    }

    bool removeRow(int row)
    {
        if(m_locked)
        {
            return false;
        }
        if(row < 0 || row >= rowCount())
        {
            return false;
        }
        beginRemoveRows(QModelIndex(), row, row);
        m_servers.removeAt(row);
        endRemoveRows(); // does absolutely nothing, the selected server stays as the next line...
        scheduleSave();
        return true;
    }

    bool moveUp(int row)
    {
        if(m_locked)
        {
            return false;
        }
        if(row <= 0)
        {
            return false;
        }
        beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        m_servers.swapItemsAt(row-1, row);
#else
        m_servers.swap(row-1, row);
#endif
        endMoveRows();
        scheduleSave();
        return true;
    }

    bool moveDown(int row)
    {
        if(m_locked)
        {
            return false;
        }
        int count = rowCount();
        if(row + 1 >= count)
        {
            return false;
        }
        beginMoveRows(QModelIndex(), row, row, QModelIndex(), row + 2);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        m_servers.swapItemsAt(row+1, row);
#else
        m_servers.swap(row+1, row);
#endif
        endMoveRows();
        scheduleSave();
        return true;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (section < 0 || section >= COLUMN_COUNT)
            return QVariant();

        if(role == Qt::DisplayRole)
        {
            switch(section)
            {
                case 0:
                    return tr("Name");
                case 1:
                    return tr("Address");
                case 2:
                    return tr("Players");
                case 3:
                    return tr("Ping");
            }
        }

        return QAbstractListModel::headerData(section, orientation, role);
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

        int row = index.row();
        int column = index.column();
        if(column < 0 || column >= COLUMN_COUNT)
            return QVariant();

        if (row < 0 || row >= m_servers.size())
            return QVariant();

        switch(column)
        {
            case 0:
                switch (role)
                {
                case Qt::DecorationRole:
                {
                    auto & bytes = m_servers[row].m_icon;
                    if(bytes.size())
                    {
                        QPixmap px;
                        if(px.loadFromData(bytes))
                            return QIcon(px);
                    }
                    return APPLICATION->getThemedIcon("unknown_server");
                }
                case Qt::DisplayRole:
                    return m_servers[row].m_name;
                case Qt::ToolTipRole:
                    if (m_servers[row].m_checked && !m_servers[row].m_motd.isEmpty())
                        return stripMinecraftFormatting(m_servers[row].m_motd);
                    return QVariant();
                case ServerPtrRole:
                    return QVariant::fromValue<void *>((void *)&m_servers[row]);
                default:
                    return QVariant();
                }
            case 1:
                switch (role)
                {
                case Qt::DisplayRole:
                    return m_servers[row].m_address;
                default:
                    return QVariant();
                }
            case 2:
                switch (role)
                {
                case Qt::DisplayRole:
                    if (!m_servers[row].m_checked)
                        return QVariant();
                    if (!m_servers[row].m_up)
                        return tr("Offline");
                    return tr("%1 / %2 players")
                        .arg(QLocale().toString(m_servers[row].m_currentPlayers))
                        .arg(QLocale().toString(m_servers[row].m_maxPlayers));
                case Qt::ForegroundRole:
                    if (!m_servers[row].m_checked || !m_servers[row].m_up)
                        return QVariant();
                    if (m_servers[row].m_currentPlayers == 0)
                        return QColor(Qt::gray);
                    if (m_servers[row].m_currentPlayers >= m_servers[row].m_maxPlayers)
                        return QColor(Qt::red);
                    return QColor(Qt::green);
                default:
                    return QVariant();
                }
            case 3:
                switch (role)
                {
                case Qt::DisplayRole:
                    if (!m_servers[row].m_checked || !m_servers[row].m_up)
                        return QVariant();
                    return tr("%1 ms").arg(m_servers[row].m_ping);
                case Qt::ForegroundRole: {
                    if (!m_servers[row].m_checked || !m_servers[row].m_up)
                        return QVariant();
                    int ping = m_servers[row].m_ping;
                    if (ping < 80)
                        return QColor(Qt::green);
                    if (ping < 150)
                        return QColor(Qt::yellow);
                    if (ping < 200)
                        return QColor(QColor(255, 165, 0)); // orange
                    return QColor(Qt::red);
                }
                default:
                    return QVariant();
                }
            default:
                return QVariant();
        }
    }

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return m_servers.size();
    }
    int columnCount(const QModelIndex & parent) const override
    {
        return COLUMN_COUNT;
    }

    Server * at(int index)
    {
        if(index < 0 || index >= rowCount())
        {
            return nullptr;
        }
        return &m_servers[index];
    }

    void setName(int row, const QString & name)
    {
        if(m_locked)
        {
            return;
        }
        auto server = at(row);
        if(!server || server->m_name == name)
        {
            return;
        }
        server->m_name = name;
        emit dataChanged(index(row, 0), index(row, COLUMN_COUNT - 1));
        scheduleSave();
    }

    void setAddress(int row, const QString & address)
    {
        if(m_locked)
        {
            return;
        }
        auto server = at(row);
        if(!server || server->m_address == address)
        {
            return;
        }
        server->m_address = address;
        emit dataChanged(index(row, 0), index(row, COLUMN_COUNT - 1));
        scheduleSave();
    }

    void setAcceptsTextures(int row, Server::AcceptsTextures textures)
    {
        if(m_locked)
        {
            return;
        }
        auto server = at(row);
        if(!server || server->m_acceptsTextures == textures)
        {
            return;
        }
        server->m_acceptsTextures = textures;
        emit dataChanged(index(row, 0), index(row, COLUMN_COUNT - 1));
        scheduleSave();
    }

    void load()
    {
        cancelSave();
        beginResetModel();
        QList<Server> servers;
        auto serversDat = parseServersDat(serversPath());
        if(serversDat)
        {
            auto &serversList = serversDat->at("servers").as<nbt::tag_list>();
            for(auto iter = serversList.begin(); iter != serversList.end(); iter++)
            {
                auto & serverTag = (*iter).as<nbt::tag_compound>();
                Server s(serverTag);
                servers.append(s);
            }
        }
        m_servers.swap(servers);
        m_loaded = true;
        endResetModel();
    }

    void saveNow()
    {
        if(saveIsScheduled())
        {
            save_internal();
        }
    }

    void pingServer(int row)
    {
        if (row < 0 || row >= m_servers.size())
            return;

        QPersistentModelIndex persistentIdx(index(row, 0));

        m_servers[row].m_checked = false;
        m_servers[row].m_up      = false;
        emit dataChanged(index(row, 0), index(row, COLUMN_COUNT - 1));

        QString addr = m_servers[row].m_address.trimmed();
        if (addr.isEmpty())
            return;

        QString host;
        quint16 port = 25565;

        if (addr.startsWith('[')) {
            // [IPv6]:port or [IPv6]
            int closeBracket = addr.indexOf(']');
            if (closeBracket != -1) {
                host = addr.mid(1, closeBracket - 1);
                if (addr.size() > closeBracket + 2 && addr[closeBracket + 1] == ':') {
                    bool ok;
                    int p = addr.mid(closeBracket + 2).toInt(&ok);
                    if (ok && p > 0 && p <= 65535)
                        port = static_cast<quint16>(p);
                }
            } else {
                host = addr;
            }
        } else if (addr.count(':') > 1) {
            // Bare IPv6 address, no port
            host = addr;
        } else {
            // hostname:port or hostname
            int colonIdx = addr.lastIndexOf(':');
            if (colonIdx != -1) {
                bool ok;
                int p = addr.mid(colonIdx + 1).toInt(&ok);
                if (ok && p > 0 && p <= 65535) {
                    host = addr.left(colonIdx);
                    port = static_cast<quint16>(p);
                } else {
                    host = addr;
                }
            } else {
                host = addr;
            }
        }

        auto* pinger = new ServerPinger(host, port, this);
        connect(pinger, &ServerPinger::done, this,
            [this, persistentIdx](bool success, int current, int max, int ping, const QString& motd, const QByteArray& icon) {
                if (!persistentIdx.isValid())
                    return;
                int r = persistentIdx.row();
                m_servers[r].m_checked        = true;
                m_servers[r].m_up             = success;
                m_servers[r].m_currentPlayers = current;
                m_servers[r].m_maxPlayers     = max;
                m_servers[r].m_ping           = ping;
                m_servers[r].m_motd           = motd;
                if (!icon.isEmpty())
                    m_servers[r].m_icon = icon;
                emit dataChanged(index(r, 0), index(r, COLUMN_COUNT - 1));
            });
        pinger->ping();
    }

    void pingAll()
    {
        for (int i = 0; i < m_servers.size(); i++)
            pingServer(i);
    }

public slots:
    void dirChanged(const QString& path)
    {
        qDebug() << "Changed:" << path;
        load();
    }
    void fileChanged(const QString& path)
    {
        qDebug() << "Changed:" << path;
    }

private slots:
    void save_internal()
    {
        cancelSave();
        QString path = serversPath();
        qDebug() << "Server list about to be saved to" << path;

        nbt::tag_compound out;
        nbt::tag_list list;
        for(auto & server: m_servers)
        {
            nbt::tag_compound serverNbt;
            server.serialize(serverNbt);
            list.push_back(std::move(serverNbt));
        }
        out.insert("servers", nbt::value(std::move(list)));

        if(!serializeServerDat(path, &out))
        {
            qDebug() << "Failed to save server list:" << path << "Will try again.";
            scheduleSave();
        }
    }

private:
    void scheduleSave()
    {
        if(!m_loaded)
        {
            qDebug() << "Server list should never save if it didn't successfully load, path:" << m_path;
            return;
        }
        if(!m_dirty)
        {
            m_dirty = true;
            qDebug() << "Server list save is scheduled for" << m_path;
        }
        m_saveTimer.start();
    }

    void cancelSave()
    {
        m_dirty = false;
        m_saveTimer.stop();
    }

    bool saveIsScheduled() const
    {
        return m_dirty;
    }

    void updateFSObserver()
    {
        bool observingFS = m_watcher->directories().contains(m_path);
        if(m_observed && m_locked)
        {
            if(!observingFS)
            {
                qWarning() << "Will watch" << m_path;
                if(!m_watcher->addPath(m_path))
                {
                    qWarning() << "Failed to start watching" << m_path;
                }
            }
        }
        else
        {
            if(observingFS)
            {
                qWarning() << "Will stop watching" << m_path;
                if(!m_watcher->removePath(m_path))
                {
                    qWarning() << "Failed to stop watching" << m_path;
                }
            }
        }
    }

    QString serversPath()
    {
        QFileInfo foo(FS::PathCombine(m_path, "servers.dat"));
        return foo.filePath();
    }

private:
    bool m_loaded = false;
    bool m_locked = false;
    bool m_observed = false;
    bool m_dirty = false;
    QString m_path;
    QList<Server> m_servers;
    QFileSystemWatcher *m_watcher = nullptr;
    QTimer m_saveTimer;
};

ServersPage::ServersPage(InstancePtr inst, QWidget* parent)
    : QMainWindow(parent), ui(new Ui::ServersPage)
{
    ui->setupUi(this);
    m_inst = inst;
    m_model = new ServersModel(inst->gameRoot(), this);
    ui->serversView->setIconSize(QSize(64,64));
    ui->serversView->setModel(m_model);
    ui->serversView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->serversView, &QTreeView::customContextMenuRequested, this, &ServersPage::ShowContextMenu);

    auto head = ui->serversView->header();
    if(head->count())
    {
        head->setSectionResizeMode(0, QHeaderView::Stretch);
        for(int i = 1; i < head->count(); i++)
        {
            head->setSectionResizeMode(i, QHeaderView::ResizeToContents);
        }
    }

    auto selectionModel = ui->serversView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &ServersPage::currentChanged);
    connect(m_inst.get(), &MinecraftInstance::runningStatusChanged, this, &ServersPage::runningStateChanged);
    connect(ui->nameLine, &QLineEdit::textEdited, this, &ServersPage::nameEdited);
    connect(ui->addressLine, &QLineEdit::textEdited, this, &ServersPage::addressEdited);
    connect(ui->resourceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(resourceIndexChanged(int)));
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, &ServersPage::rowsRemoved);

    m_pingDebounce.setSingleShot(true);
    m_pingDebounce.setInterval(1500);
    connect(&m_pingDebounce, &QTimer::timeout, this, [this]() {
        if (m_pingDebounceTarget.isValid())
            m_model->pingServer(m_pingDebounceTarget.row());
    });

    m_locked = m_inst->isRunning();
    if(m_locked)
    {
        m_model->lock();
    }

    updateState();
}

ServersPage::~ServersPage()
{
    m_model->saveNow();
    delete ui;
}

void ServersPage::retranslate()
{
    ui->retranslateUi(this);
}

void ServersPage::ShowContextMenu(const QPoint& pos)
{
    auto menu = ui->toolBar->createContextMenu(this, tr("Context menu"));
    menu->exec(ui->serversView->mapToGlobal(pos));
    delete menu;
}

QMenu * ServersPage::createPopupMenu()
{
    QMenu* filteredMenu = QMainWindow::createPopupMenu();
    filteredMenu->removeAction( ui->toolBar->toggleViewAction() );
    return filteredMenu;
}

void ServersPage::runningStateChanged(bool running)
{
    if(m_locked == running)
    {
        return;
    }
    m_locked = running;
    if(m_locked)
    {
        m_model->lock();
    }
    else
    {
        m_model->unlock();
    }
    updateState();
}

void ServersPage::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    int nextServer = -1;
    if (!current.isValid())
    {
        nextServer = -1;
    }
    else
    {
        nextServer = current.row();
    }
    currentServer = nextServer;
    updateState();
}

// WARNING: this is here because currentChanged is not accurate when removing rows. the current item needs to be fixed up after removal.
void ServersPage::rowsRemoved(const QModelIndex& parent, int first, int last)
{
    if(currentServer < first)
    {
        // current was before the removal
        return;
    }
    else if(currentServer >= first && currentServer <= last)
    {
        // current got removed...
        return;
    }
    else
    {
        // current was past the removal
        int count = last - first + 1;
        currentServer -= count;
    }
}

void ServersPage::nameEdited(const QString& name)
{
    m_model->setName(currentServer, name);
}

void ServersPage::addressEdited(const QString& address)
{
    m_model->setAddress(currentServer, address);
    if (!address.trimmed().isEmpty()) {
        m_pingDebounceTarget = m_model->index(currentServer, 0);
        m_pingDebounce.start();
    }
}

void ServersPage::resourceIndexChanged(int index)
{
    auto acceptsTextures = Server::AcceptsTextures(index);
    m_model->setAcceptsTextures(currentServer, acceptsTextures);
}

void ServersPage::updateState()
{
    auto server = m_model->at(currentServer);

    bool serverEditEnabled = server && !m_locked;
    ui->addressLine->setEnabled(serverEditEnabled);
    ui->nameLine->setEnabled(serverEditEnabled);
    ui->resourceComboBox->setEnabled(serverEditEnabled);
    ui->actionMove_Down->setEnabled(serverEditEnabled);
    ui->actionMove_Up->setEnabled(serverEditEnabled);
    ui->actionRemove->setEnabled(serverEditEnabled);
    ui->actionJoin->setEnabled(serverEditEnabled);

    if(server)
    {
        ui->addressLine->setText(server->m_address);
        ui->nameLine->setText(server->m_name);
        ui->resourceComboBox->setCurrentIndex(int(server->m_acceptsTextures));
    }
    else
    {
        ui->addressLine->setText(QString());
        ui->nameLine->setText(QString());
        ui->resourceComboBox->setCurrentIndex(0);
    }

    ui->actionAdd->setDisabled(m_locked);
}

void ServersPage::openedImpl()
{
    m_model->observe();
    m_model->pingAll();
}

void ServersPage::closedImpl()
{
    m_model->unobserve();
}

void ServersPage::on_actionAdd_triggered()
{
    int position = m_model->addEmptyRow(currentServer + 1);
    if(position < 0)
    {
        return;
    }
    // select the new row
    ui->serversView->selectionModel()->setCurrentIndex(
        m_model->index(position),
        QItemSelectionModel::SelectCurrent | QItemSelectionModel::Clear | QItemSelectionModel::Rows
    );
    currentServer = position;
}

void ServersPage::on_actionRemove_triggered()
{
    m_model->removeRow(currentServer);
}

void ServersPage::on_actionMove_Up_triggered()
{
    if(m_model->moveUp(currentServer))
    {
        currentServer --;
    }
}

void ServersPage::on_actionMove_Down_triggered()
{
    if(m_model->moveDown(currentServer))
    {
        currentServer ++;
    }
}

void ServersPage::on_actionJoin_triggered()
{
    const auto &address = m_model->at(currentServer)->m_address;
    APPLICATION->launch(m_inst, true, false, nullptr, std::make_shared<MinecraftServerTarget>(MinecraftServerTarget::parse(address)));
}

void ServersPage::on_actionRefresh_triggered()
{
    m_model->pingAll();
}

#include "ServersPage.moc"
