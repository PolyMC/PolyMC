#pragma once

#include <cstdint>

namespace SystemInformation
{

struct KernelInfo
{
    // Where periods separate the respective struct values

    // 10.0.20348.0 (Windows Server 2022)
    // 6.18.0.64 (Linux 6.18.64)

    uint32_t majorVersion;
    uint32_t minorVersion;
    uint32_t buildVersion;
    uint32_t patchVersion;
};

uint64_t getSystemMemoryInBytes();
uint64_t getSystemMemoryInMiB();

KernelInfo getKernelInfo();

}; // namespace SystemInformation
