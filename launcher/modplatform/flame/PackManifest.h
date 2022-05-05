#pragma once

#include <QString>
#include <QUrl>
#include <QVector>

namespace Flame {
struct File {
    // NOTE: throws JSONValidationError
    bool parseFromBytes(const QByteArray& fileData);

    int projectId = 0;
    int fileId = 0;
    // NOTE: the opposite to 'optional'. This is at the time of writing unused.
    bool required = true;

    // our
    bool resolved = false;
    QString fileName;
    QUrl url;
    QString targetFolder = QLatin1Literal("mods");
    enum class Type { Unknown, Folder, Ctoc, SingleFile, Cmod2, Modpack, Mod } type = Type::Mod;
};

struct Modloader {
    QString id;
    bool primary = false;
};

struct Minecraft {
    QString version;
    QString libraries;
    QVector<Flame::Modloader> modLoaders;
};

struct Manifest {
    QString manifestType;
    int manifestVersion = 0;
    Flame::Minecraft minecraft;
    QString name;
    QString version;
    QString author;
    QVector<Flame::File> files;
    QString overrides;
};

void loadManifest(Flame::Manifest& m, const QString& filepath);
}  // namespace Flame
