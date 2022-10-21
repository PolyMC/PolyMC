#pragma once

#include "BaseVersion.h"
#include "JavaVersion.h"

struct JavaInstall : public BaseVersion
{
    JavaInstall(){}
    JavaInstall(const QString & id, const QString & arch, const QString & path) : id(id), arch(arch), path(path) {}

    virtual QString descriptor() override
    {
        return id.toString();
    }

    virtual QString name() override
    {
        return id.toString();
    }

    virtual QString typeString() const override
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

typedef std::shared_ptr<JavaInstall> JavaInstallPtr;
