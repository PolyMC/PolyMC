#include <QString>
#ifdef Q_OS_MACOS
#include <sys/sysctl.h>
#endif

namespace SysInfo {
    QString currentSystem();
    QString currentArch();
}

