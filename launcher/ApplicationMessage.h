#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>

struct ApplicationMessage {
    QString command;
    QMap<QString, QString> args;

    QByteArray serialize();
    void parse(const QByteArray & input);
};
