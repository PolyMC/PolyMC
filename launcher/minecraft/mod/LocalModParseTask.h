#pragma once
#include "Mod.h"
#include "ModDetails.h"
#include <QDebug>
#include <QObject>
#include <QRunnable>

class LocalModParseTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    struct Result {
        QString id;
        std::shared_ptr<ModDetails> details;
    };
    using ResultPtr = std::shared_ptr<Result>;
    ResultPtr result() const {
        return m_result;
    }

    LocalModParseTask(int token, Mod::ModType type, const QFileInfo & modFile);
    void run() override;

signals:
    void finished(int token);

private:
    void processAsZip();
    void processAsFolder();
    void processAsLitemod();

private:
    int m_token;
    Mod::ModType m_type;
    QFileInfo m_modFile;
    ResultPtr m_result;
};
