#include <QWidget>

#include "WinDarkmode.h"

namespace WinDarkmode {

template<int syscall_id, typename... arglist> __attribute((naked)) uint32_t __fastcall WinSyscall([[maybe_unused]] arglist... args)
{
    asm volatile("mov %%rcx, %%r10; movl %0, %%eax; syscall; ret"
            :: "i"(syscall_id));
}

VOID ApplyStringProp(HWND hWnd, LPCWSTR lpString, WORD Property)
{
    WORD Prop = (uint16_t)(uint64_t)GetPropW(hWnd, (LPCWSTR)(uint64_t)Property);
    if (Prop)
    {
        DeleteAtom(Prop);
        RemovePropW(hWnd, (LPCWSTR)(uint64_t)Property);
    }
    if (lpString)
    {
        ATOM v = AddAtomW(lpString);
        if (v)
            SetPropW(hWnd, (LPCWSTR)(uint64_t)Property, (HANDLE)(uint64_t)v);
    }
}

VOID AllowDarkModeForWindow(HWND hWnd, BOOL Enable)
{
    if (hWnd)
    {
        ApplyStringProp(hWnd, Enable ? L"Enabled" : NULL, 0xA91E);
    }
    return;
}

BOOL IsWindows11()
{
    HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
    return GetProcAddress(hKern32, "Wow64SetThreadDefaultGuestMachine") != NULL; // Win11 21h2+
}

BOOL IsWindows10_Only()
{
    HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
    HMODULE hNtuser = GetModuleHandleW(L"ntdll.dll");
    return GetProcAddress(hKern32, "SetThreadSelectedCpuSets") != NULL
            && GetProcAddress(hNtuser, "ZwSetInformationCpuPartition") == NULL; 
}

BOOL IsWindows8_0_Only()
{
    HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
    return GetProcAddress(hKern32, "CreateFile2") != NULL //  Export added in 6.2 (8)
        && GetProcAddress(hKern32, "AppXFreeMemory") != NULL;  // Export added in 6.2 (8), removed in 6.3 (8.1)
}

BOOL IsWindows8_1_Only()
{
    HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
    return GetProcAddress(hKern32, "CalloutOnFiberStack") != NULL //  Export added in 6.3 (8.1), Removed in 10.0.10586
        && GetProcAddress(hKern32, "SetThreadSelectedCpuSets") == NULL; // Export added in 10.0 (10)
}

void setWindowDarkModeEnabled(HWND hWnd, bool Enabled)
{
    AllowDarkModeForWindow(hWnd, Enabled);
    BOOL DarkEnabled = (BOOL)Enabled;
    WINDOWCOMPOSITIONATTRIBDATA data = {
        WCA_USEDARKMODECOLORS,
        &DarkEnabled,
        sizeof(DarkEnabled)
    };

#ifdef _WIN64
    constexpr int NtUserSetWindowCompositionAttribute_NT6_2 = 0x13b4;
    constexpr int NtUserSetWindowCompositionAttribute_NT6_3 = 0x13e5;

    if (IsWindows8_0_Only())
        WinSyscall<NtUserSetWindowCompositionAttribute_NT6_2>(hWnd, &data);
    else if (IsWindows8_1_Only())
        WinSyscall<NtUserSetWindowCompositionAttribute_NT6_3>(hWnd, &data);
    else if (IsWindows10_Only() || IsWindows11())
    {
        ((fnSetWindowCompositionAttribute)(PVOID)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"))
            (hWnd, &data);
        // Verified this ordinal is the same through Win11 22H2 (5/8/2023)
        ((fnSetPreferredAppMode)(PVOID)GetProcAddress(GetModuleHandleW(L"uxtheme.dll"), MAKEINTRESOURCEA(135)))
            (AppMode_AllowDark);
    }
#else
    if (IsWindows10_Only())
    {
        ((fnSetWindowCompositionAttribute)(PVOID)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"))
            (hWnd, &data);
        // Verified this ordinal is the same through Win11 22H2 (5/8/2023)
        ((fnSetPreferredAppMode)(PVOID)GetProcAddress(GetModuleHandleW(L"uxtheme.dll"), MAKEINTRESOURCEA(135)))
            (AppMode_AllowDark);
    }
#endif

}

}
