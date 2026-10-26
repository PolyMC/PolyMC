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
#endif

#if defined(Q_OS_WIN)
using NtProcessFunction = LONG(NTAPI*)(HANDLE);

static NtProcessFunction ntProcessFunction(const char* name)
{
    auto ntdll = GetModuleHandleW(L"ntdll.dll");
    return ntdll ? reinterpret_cast<NtProcessFunction>(GetProcAddress(ntdll, name)) : nullptr;
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
