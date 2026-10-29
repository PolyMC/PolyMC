/*
 * PROJECT:     PolyMC Minecraft launcher
 * SPDX-License-Identifier: GPL-3.0-only
 * PURPOSE:     Legacy C allocator provider
 */

#include "CAllocator.h"

#include <algorithm>
#include <cstring>
#include <new>

void* CAllocator::allocate(void* udata, void* ptr, size_t oldsize, size_t newsize)
{
    (void)udata;
    if (newsize == 0)
    {
        delete[] static_cast<char*>(ptr);
        return nullptr;
    }

    char* block = new (std::nothrow) char[newsize];
    if (block == nullptr)
        return nullptr;

    if (ptr != nullptr)
    {
        std::memcpy(block, ptr, std::min(oldsize, newsize));
        delete[] static_cast<char*>(ptr);
    }
    return block;
}
