/*
 * PROJECT:     PolyMC Minecraft launcher
 * SPDX-License-Identifier: GPL-3.0-only
 * PURPOSE:     Legacy C allocator provider
 */

#pragma once

#include <cstddef>

class CAllocator
{
public:
    static void* allocate(void* udata, void* ptr, size_t oldsize, size_t newsize);
};
