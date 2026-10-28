// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ProcessSuspender.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>
#ifndef SYS_pidfd_send_signal
#define SYS_pidfd_send_signal 424
#endif
#ifndef SYS_pidfd_open
#define SYS_pidfd_open 434
#endif
#elif defined(Q_OS_MACOS)
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <dlfcn.h>
#include <libproc.h>
#include <mach/message.h>
#include <unistd.h>
#elif defined(Q_OS_FREEBSD)
#include <csignal>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <unistd.h>
#elif defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD)
#include <csignal>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#endif

#if defined(Q_OS_WIN)
using NtProcessFunction = LONG(NTAPI*)(HANDLE);

static NtProcessFunction ntProcessFunction(const char* name)
{
    auto ntdll = GetModuleHandleW(L"ntdll.dll");
    return ntdll ? reinterpret_cast<NtProcessFunction>(reinterpret_cast<void (*)()>(GetProcAddress(ntdll, name))) : nullptr;
}

static NtProcessFunction ntSuspendProcess()
{
    static const auto function = ntProcessFunction("NtSuspendProcess");
    return function;
}

static NtProcessFunction ntResumeProcess()
{
    static const auto function = ntProcessFunction("NtResumeProcess");
    return function;
}
#elif defined(Q_OS_LINUX)
static qint64 parentPid(qint64 pid)
{
    char path[64];
    std::snprintf(path, sizeof(path), "/proc/%lld/stat", static_cast<long long>(pid));
    int fd = ::open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return -1;

    char buffer[1024];
    auto size = ::read(fd, buffer, sizeof(buffer) - 1);
    ::close(fd);
    if (size <= 0)
        return -1;
    buffer[size] = '\0';

    auto* name_end = std::strrchr(buffer, ')');
    char state;
    long long ppid;
    if (!name_end || std::sscanf(name_end + 1, " %c %lld", &state, &ppid) != 2)
        return -1;
    return ppid;
}

static bool sendPidfdSignal(int pidfd, int signal)
{
    return syscall(SYS_pidfd_send_signal, pidfd, signal, nullptr, 0) == 0;
}
#elif defined(Q_OS_MACOS)
struct UniqueIdentifierInfo {
    uint8_t uuid[16];
    uint64_t unique_id;
    uint64_t parent_unique_id;
    int32_t id_version;
    int32_t original_parent_id_version;
    uint64_t reserved[2];
};
static_assert(sizeof(UniqueIdentifierInfo) == 56, "PROC_PIDUNIQIDENTIFIERINFO returns 56 bytes");

static constexpr int ProcPidUniqueIdentifierInfo = 17;

static bool uniqueIdentifierInfo(qint64 pid, UniqueIdentifierInfo& info)
{
    return proc_pidinfo(static_cast<int>(pid), ProcPidUniqueIdentifierInfo, 0, &info, sizeof(info)) == static_cast<int>(sizeof(info));
}

using SignalWithAuditToken = int (*)(audit_token_t*, int);

static SignalWithAuditToken signalWithAuditToken()
{
    static const auto function = reinterpret_cast<SignalWithAuditToken>(dlsym(RTLD_DEFAULT, "proc_signal_with_audittoken"));
    return function;
}
#elif defined(Q_OS_FREEBSD)
using ProcessInfo = kinfo_proc;

static constexpr char ZombieState = 5;

static bool processInfo(qint64 pid, kinfo_proc& info)
{
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, static_cast<int>(pid) };
    size_t size = sizeof(info);
    return sysctl(mib, 4, &info, &size, nullptr, 0) == 0 && size == sizeof(info) && info.ki_structsize == sizeof(info);
}

static quint64 startTime(const kinfo_proc& info)
{
    if (info.ki_stat == ZombieState)
        return 0;
    return static_cast<quint64>(info.ki_start.tv_sec) * 1000000 + static_cast<quint64>(info.ki_start.tv_usec);
}

static qint64 parentPid(const kinfo_proc& info)
{
    return info.ki_ppid;
}
#elif defined(Q_OS_OPENBSD)
using ProcessInfo = kinfo_proc;

static constexpr quint32 ProcessExiting = 0x00000008;
static constexpr quint32 ProcessZombie = 0x00040000;

static bool processInfo(qint64 pid, kinfo_proc& info)
{
    int mib[6] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, static_cast<int>(pid), sizeof(kinfo_proc), 1 };
    size_t size = sizeof(info);
    return sysctl(mib, 6, &info, &size, nullptr, 0) == 0 && size == sizeof(info);
}

static quint64 startTime(const kinfo_proc& info)
{
    if (info.p_psflags & (ProcessExiting | ProcessZombie))
        return 0;
    return info.p_ustart_sec * 1000000 + info.p_ustart_usec;
}

static qint64 parentPid(const kinfo_proc& info)
{
    return info.p_ppid;
}
#elif defined(Q_OS_NETBSD)
using ProcessInfo = kinfo_proc2;

static constexpr quint64 DyingState = 3;
static constexpr quint64 ZombieState = 5;
static constexpr quint64 DeadState = 6;

static bool processInfo(qint64 pid, kinfo_proc2& info)
{
    int mib[6] = { CTL_KERN, KERN_PROC2, KERN_PROC_PID, static_cast<int>(pid), sizeof(kinfo_proc2), 1 };
    size_t size = sizeof(info);
    return sysctl(mib, 6, &info, &size, nullptr, 0) == 0 && size == sizeof(info);
}

static quint64 startTime(const kinfo_proc2& info)
{
    if (info.p_realstat == DyingState || info.p_realstat == ZombieState || info.p_realstat == DeadState)
        return 0;
    return static_cast<quint64>(info.p_ustart_sec) * 1000000 + info.p_ustart_usec;
}

static qint64 parentPid(const kinfo_proc2& info)
{
    return info.p_ppid;
}
#endif

ProcessSuspender::~ProcessSuspender()
{
    detach();
}

ProcessSuspender::Process ProcessSuspender::openProcess(qint64 pid)
{
    Process process{ pid, InvalidHandle };
#if defined(Q_OS_WIN)
    if (ntSuspendProcess() && ntResumeProcess())
        process.handle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, static_cast<DWORD>(pid));
#elif defined(Q_OS_LINUX)
    process.handle = static_cast<int>(syscall(SYS_pidfd_open, static_cast<pid_t>(pid), 0));
#elif defined(Q_OS_MACOS)
    UniqueIdentifierInfo info;
    if (signalWithAuditToken() && uniqueIdentifierInfo(pid, info))
        process.handle = info.unique_id;
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD)
    ProcessInfo info;
    if (processInfo(pid, info))
        process.handle = startTime(info);
#endif
    return process;
}

void ProcessSuspender::closeProcess(const Process& process)
{
#if defined(Q_OS_WIN)
    CloseHandle(process.handle);
#elif defined(Q_OS_LINUX)
    ::close(process.handle);
#else
    Q_UNUSED(process);
#endif
}

bool ProcessSuspender::isOwnChild(const Process& process)
{
#if defined(Q_OS_WIN)
    Q_UNUSED(process);
    return true;
#elif defined(Q_OS_LINUX)
    return parentPid(process.pid) == getpid() && sendPidfdSignal(process.handle, 0);
#elif defined(Q_OS_MACOS)
    UniqueIdentifierInfo self, info;
    return uniqueIdentifierInfo(getpid(), self) && uniqueIdentifierInfo(process.pid, info) &&
           info.unique_id == process.handle && info.parent_unique_id == self.unique_id;
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD)
    ProcessInfo info;
    return processInfo(process.pid, info) && startTime(info) == process.handle && parentPid(info) == getpid();
#else
    Q_UNUSED(process);
    return false;
#endif
}

bool ProcessSuspender::sendSignal(const Process& process, bool stop)
{
#if defined(Q_OS_WIN)
    return (stop ? ntSuspendProcess() : ntResumeProcess())(process.handle) >= 0;
#elif defined(Q_OS_LINUX)
    return sendPidfdSignal(process.handle, stop ? SIGSTOP : SIGCONT);
#elif defined(Q_OS_MACOS)
    for (int attempt = 0; attempt < 3; attempt++) {
        UniqueIdentifierInfo info;
        if (!uniqueIdentifierInfo(process.pid, info) || info.unique_id != process.handle)
            return false;

        audit_token_t token{};
        token.val[5] = static_cast<unsigned int>(process.pid);
        token.val[7] = static_cast<unsigned int>(info.id_version);
        int error = signalWithAuditToken()(&token, stop ? SIGSTOP : SIGCONT);
        if (error != ESRCH)
            return error == 0;
    }
    return false;
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD)
    ProcessInfo info;
    if (!processInfo(process.pid, info) || startTime(info) != process.handle)
        return false;
    return ::kill(static_cast<pid_t>(process.pid), stop ? SIGSTOP : SIGCONT) == 0;
#else
    Q_UNUSED(process);
    Q_UNUSED(stop);
    return false;
#endif
}

bool ProcessSuspender::attach(qint64 pid)
{
    detach();
    if (pid <= 0)
        return false;

    auto process = openProcess(pid);
    if (process.handle == InvalidHandle)
        return false;

    if (!isOwnChild(process)) {
        closeProcess(process);
        return false;
    }

    m_process = process;
    return true;
}

void ProcessSuspender::detach()
{
    resume();
    release();
}

void ProcessSuspender::release()
{
    if (isAttached())
        closeProcess(m_process);
    m_process = {};
    m_suspended = false;
}

bool ProcessSuspender::suspend()
{
    if (!isAttached() || m_suspended)
        return false;

    m_suspended = sendSignal(m_process, true);
    return m_suspended;
}

bool ProcessSuspender::resume()
{
    if (!m_suspended)
        return false;

    m_suspended = false;
    return sendSignal(m_process, false);
}
