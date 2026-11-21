#pragma once

#include <QIcon>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

class LogoCache : public QObject
{
    Q_OBJECT

public:
    enum class Skip
    {
        Nothing,
        EmptyUrl,
        NullName
    };

    LogoCache(QString base, QString job, QString placeholder, Skip skip,
              QObject* parent);

    QIcon icon(const QString& name, const QString& url);
    QString path(const QString& name, const QString& url);

signals:
    void logoLoaded(const QString& name);

private:
    bool skips(const QString& name, const QString& url) const;
    void request(const QString& name, const QString& url);
    void onDownloaded(const QString& name, const QString& file);
    void onFailed(const QString& name);

    QString m_base;
    QString m_job;
    QString m_placeholder;
    Skip m_skip;
    QMap<QString, QIcon> m_icons;
    QStringList m_loading;
    QStringList m_failed;
};
