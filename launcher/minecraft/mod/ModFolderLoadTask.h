#pragma once
#include "Mod.h"
#include <QDir>
#include <QMap>
#include <QObject>
#include <QRunnable>
#include <memory>

class ModFolderLoadTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    struct Result {
        QMap<QString, Mod> mods;
    };
    using ResultPtr = std::shared_ptr<Result>;
    ResultPtr result() const {
        return m_result;
    }

public:
    ModFolderLoadTask(QDir dir);
    void run() override;
signals:
    void succeeded();
private:
    QDir m_dir;
    ResultPtr m_result;
};
