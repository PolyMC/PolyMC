/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once
#include <QObject>
#include <QString>
#include <memory>


class QLocalServer;
class LockedFile;

class ApplicationId
{
public: /* methods */
    // traditional app = installed system wide and used in a multi-user environment
    static ApplicationId fromTraditionalApp();
    // ID based on a path with all the application data (no two instances with the same data path should run)
    static ApplicationId fromPathAndVersion(const QString & dataPath, const QString & version);
    // custom ID
    static ApplicationId fromCustomId(const QString & id);
    // custom ID, based on a raw string previously acquired from 'toString'
    static ApplicationId fromRawString(const QString & id);


    QString toString()
    {
        return m_id;
    }

private: /* methods */
    ApplicationId(const QString & value)
    {
        m_id = value;
    }

private: /* data */
    QString m_id;
};

class LocalPeer : public QObject
{
    Q_OBJECT

public:
    LocalPeer(QObject *parent, const ApplicationId &appId);
    ~LocalPeer() override;
    bool isClient();
    bool sendMessage(const QByteArray &message, int timeout);
    ApplicationId applicationId() const;

Q_SIGNALS:
    void messageReceived(const QByteArray &message);

protected Q_SLOTS:
    void receiveConnection();

protected:
    ApplicationId id;
    QString socketName;
    std::unique_ptr<QLocalServer> server;
    std::unique_ptr<LockedFile> lockFile;
};
