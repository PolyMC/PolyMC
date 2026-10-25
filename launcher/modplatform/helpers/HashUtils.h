#pragma once

#include <QFutureWatcher>
#include <QString>

#include "modplatform/ModIndex.h"
#include "tasks/Task.h"

namespace Hashing {

class Hasher : public Task {
   public:
    using Ptr = shared_qobject_ptr<Hasher>;
    using HashFunction = QString (*)(const QString&);

    Hasher(QString file_path, HashFunction hash_function);

    /* We can't really abort this task, but we can say we aborted and finish our thing quickly :) */
    bool abort() override
    {
        if (isRunning())
            emitAborted();
        return true;
    }

    void executeTask() override;

    QString computeHash() const { return m_hash_function(m_path); }

    QString getResult() const { return m_hash; };
    QString getPath() const { return m_path; };

   protected:
    QString m_hash;
    QString m_path;

   private:
    HashFunction m_hash_function;
    QFutureWatcher<QString> m_watcher;
};

class FlameHasher : public Hasher {
   public:
    FlameHasher(QString file_path);
};

class ModrinthHasher : public Hasher {
   public:
    ModrinthHasher(QString file_path);
};

Hasher::Ptr createHasher(QString file_path, ModPlatform::Provider provider);
Hasher::Ptr createFlameHasher(QString file_path);
Hasher::Ptr createModrinthHasher(QString file_path);

}  // namespace Hashing
