#include "JavaInstall.h"
#include <MMCStrings.h>

int JavaInstall::compare(const JavaInstall& other) const
{
    auto archCompare = Strings::naturalCompare(arch, other.arch, Qt::CaseInsensitive);
    if(archCompare != 0)
        return archCompare;
    if(id < other.id)
        return -1;
    if(other.id < id)
        return 1;
    return Strings::naturalCompare(path, other.path, Qt::CaseInsensitive);
}
