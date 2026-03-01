# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

## UseCcache ##

# Adds an option to enable CCache and uses it if provided.
# Also does some debug info downgrading to make it easier.
# Credit to DraVee for his work on this

option(USE_CCACHE "Use ccache for compilation" OFF)
set(CCACHE_PATH "ccache" CACHE STRING "Path to ccache binary")
if(USE_CCACHE)
    find_program(CCACHE_BINARY ${CCACHE_PATH})
    if(CCACHE_BINARY)
        message(STATUS "[UseCcache] Found ccache at: ${CCACHE_BINARY}")
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_BINARY})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_BINARY})
    else()
        message(FATAL_ERROR "[UseCcache] USE_CCACHE enabled, but no "
            "executable found at: ${CCACHE_PATH}")
    endif()
    # Follow SCCache recommendations:
    # <https://github.com/mozilla/sccache/blob/main/README.md?plain=1#L144>
    if(WIN32)
        string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG
            "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG
            "${CMAKE_C_FLAGS_DEBUG}")
        string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO
            "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
        string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELWITHDEBINFO
            "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    endif()
endif()
