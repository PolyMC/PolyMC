#include <QString>
#ifdef Q_OS_MACOS
#include <sys/sysctl.h>
#endif

#ifdef Q_OS_MACOS
bool rosettaDetect() {
    int ret = 0;
    size_t size = sizeof(ret);
    if (sysctlbyname("sysctl.proc_translated", &ret, &size, NULL, 0) == -1)
    {
        return false;
    }
    if(ret == 0)
    {
        return false;
    }
    if(ret == 1)
    {
        return true;
    }
    return false;
}
#endif

namespace SysInfo {
    QString currentSystem() {
#ifdef Q_OS_LINUX
        return "linux";
#elif Q_OS_MACOS
        return "osx";
#elif Q_OS_WINDOWS
        return "windows";
#elif Q_OS_FREEBSD
        return "freebsd";
#elif Q_OS_OPENBSD
        return "openbsd";
#else
        return "unknown";
#endif
    };

    QString currentArch() {
#ifdef Q_PROCESSOR_X86_64
    #ifdef Q_OS_MACOS
        if(rosettaDetect())
        {
          return "arm64";
        }
        else
        {
          return "amd64";
        }
    #else
        return "amd64";
    #endif
#elif Q_PROCESSOR_X86_32
        return "i386";
#elif Q_PROCESSOR_ARM
        return "arm64";
#endif
    };
}

