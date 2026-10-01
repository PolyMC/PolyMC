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

// This file has been adapted from QuaZip's implementation, which is licensed
// under the LGPL 2.1 or later.

#include "PolyMCTextCodec.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

#include <QHash>

static QHash<QStringConverter::Encoding,PolyMCTextCodec*> *static_hash_quazip_codecs = nullptr;

class PolyMCTextCodecCleanup
{
public:
    explicit PolyMCTextCodecCleanup()
    {
    }
    ~PolyMCTextCodecCleanup()
    {
        if (static_hash_quazip_codecs)
        {
            QList<PolyMCTextCodec*>list_quazip_codecs = static_hash_quazip_codecs->values();
            qDeleteAll(list_quazip_codecs.begin(),list_quazip_codecs.end());
            static_hash_quazip_codecs->clear();
            delete static_hash_quazip_codecs;
            static_hash_quazip_codecs = nullptr;
        }
    }
};

Q_GLOBAL_STATIC(PolyMCTextCodecCleanup, createPolyMCTextCodecCleanup)

PolyMCTextCodec::PolyMCTextCodec()
{
}

void PolyMCTextCodec::setup()
{
    if (static_hash_quazip_codecs) return;
    (void)createPolyMCTextCodecCleanup();

    static_hash_quazip_codecs = new QHash<QStringConverter::Encoding,PolyMCTextCodec*>;
}

PolyMCTextCodec *PolyMCTextCodec::codecForName(const QByteArray &name)
{
    PolyMCTextCodec::setup();
    QStringConverter::Encoding  encoding = QStringConverter::Utf8;

    std::optional<QStringConverter::Encoding> opt_encoding = QStringConverter::encodingForName(name);
    if (opt_encoding != std::nullopt)
    {
        encoding = opt_encoding.value();
    }
    if (static_hash_quazip_codecs->contains(encoding))
    {
        return static_hash_quazip_codecs->value(encoding);
    }

    PolyMCTextCodec *codec = new PolyMCTextCodec();

    codec->mEncoding = encoding;
    static_hash_quazip_codecs->insert(encoding,codec);
    return codec;
}

PolyMCTextCodec *PolyMCTextCodec::codecForLocale()
{
    PolyMCTextCodec::setup();
    return PolyMCTextCodec::codecForName("System");
}

QByteArray PolyMCTextCodec::fromUnicode(const QString &str) const
{
    auto from = QStringEncoder(mEncoding);
    return from(str);
}

QString PolyMCTextCodec::toUnicode(const QByteArray &a) const
{
    auto to = QStringDecoder(mEncoding);
    return to(a);
}

PolyMCTextDecoder::PolyMCTextDecoder(const PolyMCTextCodec *codec) : m_decoder(codec ? codec->mEncoding : QStringConverter::System)
{
}

PolyMCTextDecoder::PolyMCTextDecoder(QStringConverter::Encoding encoding) : m_decoder(encoding)
{
}

QString PolyMCTextDecoder::toUnicode(const QByteArray &data)
{
    return m_decoder(data);
}

#endif
