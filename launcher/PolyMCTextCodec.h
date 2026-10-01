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

#pragma once

#include <QByteArray>
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
typedef QTextCodec PolyMCTextCodec;
#include <QTextDecoder>
typedef QTextDecoder PolyMCTextDecoder;
#else
#include <QStringConverter>
#include <QStringDecoder>
#include <QStringEncoder>

class PolyMCTextCodec
{
public:
    explicit PolyMCTextCodec();

    QByteArray fromUnicode(const QString &str) const;
    QString toUnicode(const QByteArray &a) const;

    QStringConverter::Encoding encoding() const { return mEncoding; }

    static PolyMCTextCodec *codecForName(const QByteArray &name);
    static PolyMCTextCodec *codecForLocale();
protected:
    static void setup();
    QStringConverter::Encoding mEncoding = QStringConverter::Utf8;

    friend class PolyMCTextDecoder;
};

class PolyMCTextDecoder
{
public:
    explicit PolyMCTextDecoder(const PolyMCTextCodec *codec);
    explicit PolyMCTextDecoder(QStringConverter::Encoding encoding = QStringConverter::System);

    QString toUnicode(const QByteArray &data);
private:
    QStringDecoder m_decoder;
};
#endif
