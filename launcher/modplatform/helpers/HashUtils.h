#pragma once

#include <QString>

#include "modplatform/ModIndex.h"
#include "tasks/Task.h"

namespace Hashing {

class Hasher : public Task {
   public:
    using Ptr = shared_qobject_ptr<Hasher>;

    Hasher(QString file_path) : m_path(std::move(file_path)) {}

    /* We can't really abort this task, but we can say we aborted and finish our thing quickly :) */
    bool abort() override { return true; }

    void executeTask() override = 0;

    QString getResult() const { return m_hash; };
    QString getPath() const { return m_path; };

   protected:
    QString m_hash;
    QString m_path;
};

class FlameHasher : public Hasher {
   public:
    explicit FlameHasher(const QString& file_path) : Hasher(file_path) { setObjectName(QString("FlameHasher: %1").arg(file_path)); }

    void executeTask() override;
};

class ModrinthHasher : public Hasher {
   public:
    explicit ModrinthHasher(const QString& file_path) : Hasher(file_path) { setObjectName(QString("ModrinthHasher: %1").arg(file_path)); }

    void executeTask() override;
};

Hasher::Ptr createHasher(const QString& file_path, ModPlatform::Provider provider);
Hasher::Ptr createFlameHasher(const QString& file_path);
Hasher::Ptr createModrinthHasher(const QString& file_path);

}  // namespace Hashing
