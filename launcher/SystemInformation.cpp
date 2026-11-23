#include "SystemInformation.h"

#include <QtSystemDetection>
#include <QOperatingSystemVersion>
#include <QSysInfo>
#include <QVersionNumber>

#if defined(Q_OS_WIN32)
#include <Windows.h>
#endif

#if defined(Q_OS_LINUX)
#include <sys/sysinfo.h>
#endif

#if defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if defined(Q_OS_MACOS)
#include <sys/sysctl.h>
#endif

#if defined(Q_OS_HAIKU)
#include <OS.h>
#endif

#if defined(Q_OS_NETBSD)
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

///////////////////////////////////////////////////////////////////////////////


// Used on all except Windows and Haiku
#if !defined(Q_OS_WIN32) && !defined(Q_OS_HAIKU)
static SystemInformation::KernelInfo kernelVersionFromUname()
{
    const QVersionNumber v = QVersionNumber::fromString(QSysInfo::kernelVersion());

    SystemInformation::KernelInfo info{};
    info.majorVersion = static_cast<uint32_t>(v.segmentAt(0));
    info.minorVersion = static_cast<uint32_t>(v.segmentAt(1));
    info.patchVersion = static_cast<uint32_t>(v.segmentAt(2));
    return info;
}
#endif

#if defined(Q_OS_WIN32)
uint64_t SystemInformation::getSystemMemoryInBytes()
{
    MEMORYSTATUSEX ms{};
    ms.dwLength = sizeof(ms);

    if (!GlobalMemoryStatusEx(&ms))
    {
        return 0ull;
    }

    return static_cast<uint64_t>(ms.ullTotalPhys);
}

SystemInformation::KernelInfo SystemInformation::getKernelInfo()
{
    KernelInfo info{};

    // Qt reads using RtlGetVersion
    const QOperatingSystemVersion v = QOperatingSystemVersion::current();
    info.majorVersion = static_cast<uint32_t>(v.majorVersion());
    info.minorVersion = static_cast<uint32_t>(v.minorVersion());
    info.buildVersion = static_cast<uint32_t>(v.microVersion());
    
    return info;
}
#elif defined(Q_OS_LINUX)
uint64_t SystemInformation::getSystemMemoryInBytes()
{
    struct sysinfo si;

    if (sysinfo(&si) == -1)
    {
        return 0ull;
    }

    return static_cast<uint64_t>(si.totalram) * si.mem_unit;
}

SystemInformation::KernelInfo SystemInformation::getKernelInfo()
{
    return kernelVersionFromUname();
}
#elif defined(Q_OS_FREEBSD)
uint64_t SystemInformation::getSystemMemoryInBytes()
{
	int mib[2] = { CTL_HW, HW_PHYSMEM };
	unsigned long result = 0;

	size_t len = sizeof(result);

	if (sysctl(mib, 2, &result, &len, NULL, 0) == -1 || len != sizeof(result))
	{
        return 0ull;
    }

    return static_cast<uint64_t>(result);
}

SystemInformation::KernelInfo SystemInformation::getKernelInfo()
{
    KernelInfo info = kernelVersionFromUname();

    info.patchVersion = QSysInfo::kernelVersion().section(QStringLiteral("-p"), 1).toUInt();

    return info;
}
#elif defined(Q_OS_OPENBSD)
uint64_t SystemInformation::getSystemMemoryInBytes()
{
	int mib[2] = { CTL_HW, HW_PHYSMEM64 };
	int64_t result = 0;

    size_t len = sizeof(result);

	if (sysctl(mib, 2, &result, &len, NULL, 0) == -1 || len != sizeof(result))
	{
        return 0ull;
    }

    return static_cast<uint64_t>(result);
}

SystemInformation::KernelInfo SystemInformation::getKernelInfo()
{
    return kernelVersionFromUname();
}
#elif defined(Q_OS_MACOS)
uint64_t SystemInformation::getSystemMemoryInBytes()
{
    uint64_t result;
    size_t len = sizeof(result);

    if (sysctlbyname("hw.memsize", &result, &len, NULL, 0))
    {
        return 0ull;
    }

    return result;
}

SystemInformation::KernelInfo SystemInformation::getKernelInfo()
{
    return kernelVersionFromUname();
}
#elif defined(Q_OS_HAIKU)
uint64_t SystemInformation::getSystemMemoryInBytes()
{
    system_info info;

    if (get_system_info(&info) != B_OK)
    {
        return 0ull;
    }

    return static_cast<uint64_t>(info.max_pages) * B_PAGE_SIZE;
}

SystemInformation::KernelInfo SystemInformation::getKernelInfo()
{
    KernelInfo info{};

    system_info si;
    if (get_system_info(&si) != B_OK)
    {
        return info;
    }

    const uint64_t v = static_cast<uint64_t>(si.kernel_version);
    info.majorVersion = static_cast<uint32_t>((v >> 16) & 0xFFFF);
    info.minorVersion = static_cast<uint32_t>((v >> 8) & 0xFF);
    info.patchVersion = static_cast<uint32_t>(v & 0xFF);

    return info;
}
#elif defined(Q_OS_NETBSD)
uint64_t SystemInformation::getSystemMemoryInBytes()
{
    int mib[2] = { CTL_HW, HW_PHYSMEM64 };
    uint64_t result = 0;

    size_t len = sizeof(result);

    if (sysctl(mib, 2, &result, &len, NULL, 0) == -1 || len != sizeof(result))
    {
        return 0ull;
    }

    return result;
}

SystemInformation::KernelInfo SystemInformation::getKernelInfo()
{
    return kernelVersionFromUname();
}
#else
#error "SystemInformation.cpp requires a platform implementation"
#endif

///////////////////////////////////////////////////////////////////////////////

uint64_t SystemInformation::getSystemMemoryInMiB()
{
    constexpr uint64_t kMebibyte = 1024 * 1024;

    return getSystemMemoryInBytes() / kMebibyte;
}
