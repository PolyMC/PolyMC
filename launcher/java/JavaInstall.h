#pragma once

#include "BaseVersion.h"
#include "JavaVersion.h"

struct JavaInstall : public BaseVersion
{
    JavaInstall(){}
    JavaInstall(QString id, QString arch, QString path)
    : id(id), arch(arch), path(path)
    {
    }
    virtual QString descriptor()
    {
        return id.toString();
    }

    virtual QString name()
    {
        return id.toString();
    }

    virtual QString typeString() const
    {
        return arch;
    }

    int compare(const JavaInstall& other) const;

    JavaVersion id;
    QString arch;
    QString path;
    bool recommended = false;
};

typedef std::shared_ptr<JavaInstall> JavaInstallPtr;
