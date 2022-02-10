#pragma once

#include <utility>

#include "BaseVersion.h"
#include "JavaVersion.h"

struct JavaInstall : public BaseVersion
{
    JavaInstall() = default;
    JavaInstall(QString id, QString arch, QString path)
    : id(id), arch(std::move(arch)), path(std::move(path))
    {
    }
    QString descriptor() override
    {
        return id.toString();
    }

    QString name() override
    {
        return id.toString();
    }

    QString typeString() const override
    {
        return arch;
    }

    bool operator<(const JavaInstall & rhs);
    bool operator==(const JavaInstall & rhs);
    bool operator>(const JavaInstall & rhs);

    JavaVersion id;
    QString arch;
    QString path;
    bool recommended = false;
};

using JavaInstallPtr = std::shared_ptr<JavaInstall>;
