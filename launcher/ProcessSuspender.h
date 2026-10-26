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

#pragma once

#include <QtGlobal>

/*
 * Pauses and resumes a child process.
 *
 * The process is only ever signalled through something that pins its identity, so a process that exited can't be
 * confused with an unrelated one that reused its PID: a pidfd on Linux, a process handle on Windows, and the unique id
 * and pid version on macOS. The BSDs have none of these, so there the start time is checked right before every signal.
 * attach() has to be called while the child is still owned by its QProcess.
 */
class ProcessSuspender {
   public:
    ProcessSuspender() = default;
    ~ProcessSuspender();

    ProcessSuspender(const ProcessSuspender&) = delete;
    ProcessSuspender& operator=(const ProcessSuspender&) = delete;

    bool attach(qint64 pid);
    /** Resumes the process if it's paused, then lets go of it. */
    void detach();
    /** Lets go of a process that already exited, without signalling it. */
    void release();

    bool isAttached() const { return m_process.handle != InvalidHandle; }
    bool isSuspended() const { return m_suspended; }

    bool suspend();
    bool resume();

   private:
#if defined(Q_OS_WIN)
    using NativeHandle = void*;
    static constexpr NativeHandle InvalidHandle = nullptr;
#elif defined(Q_OS_LINUX)
    using NativeHandle = int;
    static constexpr NativeHandle InvalidHandle = -1;
#else
    using NativeHandle = quint64;
    static constexpr NativeHandle InvalidHandle = 0;
#endif

    struct Process {
        qint64 pid = 0;
        NativeHandle handle = InvalidHandle;
    };

    static Process openProcess(qint64 pid);
    static void closeProcess(const Process& process);
    static bool isOwnChild(const Process& process);
    static bool sendSignal(const Process& process, bool stop);

    Process m_process;
    bool m_suspended = false;
};
