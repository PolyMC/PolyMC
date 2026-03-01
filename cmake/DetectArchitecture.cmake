# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: LGPL-3.0-or-later

## DetectArchitecture ##
#[[
Does exactly as it sounds. Detects common symbols defined for different architectures and
adds compile definitions thereof. Namely:
- arm64
- arm
- x86_64
- x86
- ia64
- mips64
- mips
- ppc64
- ppc
- riscv
- riscv64
- loongarch64
- wasm

Unsupported architectures:
- ARMv2-6
- m68k
- PIC

This file WILL NOT detect endian-ness for you.

This file is based off of Yuzu and Dynarmic.
]]

# multiarch builds are a special case and also very difficult
# this is what I have for now, but it's not ideal

# Do note that situations where multiple architectures are defined
# should NOT be too dependent on the architecture
# otherwise, you may end up with duplicate code
if (CMAKE_OSX_ARCHITECTURES)
    set(MULTIARCH_BUILD 1)
    set(ARCHITECTURE "${CMAKE_OSX_ARCHITECTURES}")

    # hope and pray the architecture names match
    foreach(ARCH ${CMAKE_OSX_ARCHITECTURES})
        set(ARCHITECTURE_${ARCH} 1)
        add_definitions(-DARCHITECTURE_${ARCH}=1)
    endforeach()

    return()
endif()

set(MULTIARCH_BUILD 0)

include(CheckSymbolExists)
function(detect_architecture symbol arch)
    # The output variable needs to be unset between invocations otherwise
    # CMake's crazy scope rules will keep it defined
    unset(SYMBOL_EXISTS CACHE)

    if (NOT DEFINED ARCHITECTURE)
        set(CMAKE_REQUIRED_QUIET 1)
        check_symbol_exists("${symbol}" "" SYMBOL_EXISTS)
        unset(CMAKE_REQUIRED_QUIET)

        if (SYMBOL_EXISTS)
            set(ARCHITECTURE "${arch}" PARENT_SCOPE)
            set(ARCHITECTURE_${arch} 1 PARENT_SCOPE)
            add_definitions(-DARCHITECTURE_${arch}=1)
        endif()
    endif()
endfunction()

function(detect_architecture_symbols)
    if (DEFINED ARCHITECTURE)
        return()
    endif()

    set(oneValueArgs ARCH)
    set(multiValueArgs SYMBOLS)

    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "${multiValueArgs}"
        "${ARGN}")

    set(arch "${ARGS_ARCH}")
    foreach(symbol ${ARGS_SYMBOLS})
        detect_architecture("${symbol}" "${arch}")

        if (ARCHITECTURE_${arch})
            message(DEBUG "[DetectArchitecture] Found architecture symbol ${symbol} for ${arch}")
            set(ARCHITECTURE "${arch}" PARENT_SCOPE)
            set(ARCHITECTURE_${arch} 1 PARENT_SCOPE)
            add_definitions(-DARCHITECTURE_${arch}=1)

            return()
        endif()
    endforeach()
endfunction()

# arches here are put in a sane default order of importance
# notably, amd64, arm64, and riscv (in order) are BY FAR the most common
# mips is pretty popular in embedded
# ppc64 is pretty popular in supercomputing
# sparc is uh
# ia64 exists
# the rest exist, but are probably less popular than ia64

detect_architecture_symbols(
    ARCH arm64
    SYMBOLS
        "__ARM64__"
        "__aarch64__"
        "_M_ARM64")

detect_architecture_symbols(
    ARCH x86_64
    SYMBOLS
        "__x86_64"
        "__x86_64__"
        "__amd64"
        "_M_X64"
        "_M_AMD64")

# riscv is interesting since it generally does not define a riscv64-specific symbol
# We can, however, check for the rv32 zcf extension which is good enough of a heuristic on GCC
detect_architecture_symbols(
    ARCH riscv
    SYMBOLS
        "__riscv_zcf")

# if zcf doesn't exist we can safely assume it's riscv64
detect_architecture_symbols(
    ARCH riscv64
    SYMBOLS
        "__riscv")

detect_architecture_symbols(
    ARCH x86
    SYMBOLS
        "__i386"
        "__i386__"
        "_M_IX86")

detect_architecture_symbols(
    ARCH arm
    SYMBOLS
        "__arm__"
        "__TARGET_ARCH_ARM"
        "_M_ARM")

detect_architecture_symbols(
    ARCH ia64
    SYMBOLS
        "__ia64"
        "__ia64__"
        "_M_IA64")

# mips is probably the least fun to detect due to microMIPS
# Because microMIPS is such cancer I'm considering it out of scope for now
detect_architecture_symbols(
    ARCH mips64
    SYMBOLS
        "__mips64")

detect_architecture_symbols(
    ARCH mips
    SYMBOLS
        "__mips"
        "__mips__"
        "_M_MRX000")

detect_architecture_symbols(
    ARCH ppc64
    SYMBOLS
        "__ppc64__"
        "__powerpc64__"
        "_ARCH_PPC64"
        "_M_PPC64")

detect_architecture_symbols(
    ARCH ppc
    SYMBOLS
        "__ppc__"
        "__ppc"
        "__powerpc__"
        "_ARCH_COM"
        "_ARCH_PWR"
        "_ARCH_PPC"
        "_M_MPPC"
        "_M_PPC")

detect_architecture_symbols(
    ARCH sparc64
    SYMBOLS
        "__sparc_v9__")

detect_architecture_symbols(
    ARCH sparc
    SYMBOLS
        "__sparc__"
        "__sparc")

# I don't actually know about loongarch32 since crossdev does not support it, only 64
detect_architecture_symbols(
    ARCH loongarch64
    SYMBOLS
        "__loongarch__"
        "__loongarch64")

detect_architecture_symbols(
    ARCH wasm
    SYMBOLS
        "__EMSCRIPTEN__")

# "generic" target
# If you have reached this point, you're on some as-of-yet unsupported architecture.
# See the docs up above for known unsupported architectures
# If you're not in the list... I think you know what you're doing.
if (NOT DEFINED ARCHITECTURE)
    set(ARCHITECTURE "GENERIC")
    set(ARCHITECTURE_GENERIC 1)
    add_definitions(-DARCHITECTURE_GENERIC=1)
endif()

message(STATUS "[DetectArchitecture] Target architecture: ${ARCHITECTURE}")
