#include "HashUtils.h"

#include <QDebug>
#include <QFile>
#include <QtConcurrentRun>

#include <filesystem>
#include <fstream>

#include "FileSystem.h"

#include <MurmurHash2.h>

namespace Hashing {

static ModPlatform::ProviderCapabilities ProviderCaps;

Hasher::Ptr createHasher(QString file_path, ModPlatform::Provider provider)
{
    switch (provider) {
        case ModPlatform::Provider::MODRINTH:
            return createModrinthHasher(file_path);
        case ModPlatform::Provider::FLAME:
            return createFlameHasher(file_path);
        default:
            qCritical() << "[Hashing]"
                        << "Unrecognized mod platform!";
            return nullptr;
    }
}

Hasher::Ptr createModrinthHasher(QString file_path)
{
    return new ModrinthHasher(file_path);
}

Hasher::Ptr createFlameHasher(QString file_path)
{
    return new FlameHasher(file_path);
}

static QString hashModrinthFile(const QString& path)
{
    QFile file(path);

    // QFile::open() doesn't throw, it reports failure through its return value
    if (!file.open(QFile::ReadOnly)) {
        qCritical() << QString("Failed to open JAR file in %1").arg(path);
        qCritical() << QString("Reason: ") << file.errorString();
        return {};
    }

    auto hash_type = ProviderCaps.hashType(ModPlatform::Provider::MODRINTH).first();
    return ProviderCaps.hash(ModPlatform::Provider::MODRINTH, &file, hash_type);
}

static QString hashFlameFile(const QString& path)
{
    // CF-specific
    auto should_filter_out = [](char c) { return (c == 9 || c == 10 || c == 13 || c == 32); };

    // Go through std::filesystem::path, a plain narrow string can't represent every path on Windows
#ifdef Q_OS_WIN
    std::ifstream file_stream(std::filesystem::path(path.toStdWString()), std::ifstream::binary);
#else
    std::ifstream file_stream(std::filesystem::path(path.toStdString()), std::ifstream::binary);
#endif
    if (!file_stream.is_open()) {
        qCritical() << QString("Failed to open JAR file in %1").arg(path);
        return {};
    }
    return QString::number(MurmurHash2(std::move(file_stream), 4 * MiB, should_filter_out));
}

Hasher::Hasher(QString file_path, HashFunction hash_function) : m_path(std::move(file_path)), m_hash_function(hash_function)
{
    connect(&m_watcher, &QFutureWatcher<QString>::finished, this, [this] {
        if (!isRunning())
            return;
        m_hash = m_watcher.result();
        if (m_hash.isEmpty()) {
            emitFailed("Failed to hash file.");
        } else {
            emitSucceeded();
        }
    });
}

void Hasher::executeTask()
{
    m_watcher.setFuture(QtConcurrent::run(m_hash_function, m_path));
}

FlameHasher::FlameHasher(QString file_path) : Hasher(file_path, hashFlameFile)
{
    setObjectName(QString("FlameHasher: %1").arg(file_path));
}

ModrinthHasher::ModrinthHasher(QString file_path) : Hasher(file_path, hashModrinthFile)
{
    setObjectName(QString("ModrinthHasher: %1").arg(file_path));
}

}  // namespace Hashing
