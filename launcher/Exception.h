// Licensed under the Apache-2.0 license. See README.md for details.

#pragma once

#include <QString>
#include <QDebug>
#include <exception>

class Exception : public std::exception
{
public:
    Exception(const QString &message) : std::exception(), m_message(message), m_what(message.toUtf8())
    {
        qCritical() << "Exception:" << message;
    }
    Exception(const Exception &other)
        : std::exception(), m_message(other.cause()), m_what(other.m_what)
    {
    }
    virtual ~Exception() noexcept {}
    const char *what() const noexcept
    {
        return m_what.constData();
    }
    QString cause() const
    {
        return m_message;
    }

private:
    QString m_message;
    QByteArray m_what;
};
