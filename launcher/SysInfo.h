#include <QString>
#include "minecraft/LaunchContext.h"
#ifdef Q_OS_MACOS
#include <sys/sysctl.h>
#endif

namespace SysInfo {
    QString currentSystem();
    QString currentArch(LaunchContext launchContext);
    QString runCheckerForArch(LaunchContext launchContext);
    QString useQTForArch();
    QString currentOSString(LaunchContext launchContext);
    QString currentOSString(QString system, QString arch);
}

