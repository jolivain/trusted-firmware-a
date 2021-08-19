#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Developer warning: if you use cache variables from the main list files, ensure
# you add them to `CMAKE_TRY_COMPILE_PLATFORM_VARIABLES` at the bottom of the
# file. See the official CMake documentation for this variable for why.
#

set(CMAKE_SYSTEM_NAME "Generic")

#
# Make sure CMake doesn't go looking on the host system for libraries, includes
# and other packages, and doesn't look for programs at all.
#

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#
# Locate the compiler.
#

if(TFA_ARCH_STATE_AARCH32)
    find_program(CMAKE_ASM_COMPILER "arm-none-eabi-gcc"
        DOC "Path to arm-none-eabi-gcc."
        REQUIRED)

    find_program(CMAKE_C_COMPILER "arm-none-eabi-gcc"
        DOC "Path to arm-none-eabi-gcc."
        REQUIRED)
else()
    find_program(CMAKE_ASM_COMPILER "aarch64-none-elf-gcc"
        DOC "Path to aarch64-none-elf-gcc."
        REQUIRED)

    find_program(CMAKE_C_COMPILER "aarch64-none-elf-gcc"
        DOC "Path to aarch64-none-elf-gcc."
        REQUIRED)
endif()

#
# Set up compiler options.
#

foreach(lang IN ITEMS ASM C)
    string(APPEND CMAKE_${lang}_FLAGS_INIT "-march=${TFA_ARCH_LOWER} ")

    if(TFA_ARCH_STATE_AARCH32)
        string(APPEND CMAKE_${lang}_FLAGS_INIT "-mno-unaligned-access ")

        if(TFA_ARCH_STATE_AARCH32_THUMB)
            string(APPEND CMAKE_${lang}_FLAGS_INIT "-mthumb ")
        else()
            string(APPEND CMAKE_${lang}_FLAGS_INIT "-marm ")
        endif()
    else()
        string(APPEND CMAKE_${lang}_FLAGS_INIT "-mgeneral-regs-only ")
        string(APPEND CMAKE_${lang}_FLAGS_INIT "-mstrict-align ")
    endif()

    string(APPEND CMAKE_${lang}_FLAGS_INIT "-ffreestanding ")
    string(APPEND CMAKE_${lang}_FLAGS_INIT "-fdata-sections ")
    string(APPEND CMAKE_${lang}_FLAGS_INIT "-ffunction-sections ")

    string(APPEND CMAKE_${lang}_FLAGS_DEBUG_INIT "-Og ")
endforeach()

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections ")

#
# Ensure we can access some of the TF-A configuration options from the toolchain
# when CMake runs `try_compile()`.
#

list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
    TFA_ARCH_STATE_AARCH32
    TFA_ARCH_STATE_AARCH32_THUMB
    TFA_ARCH_LOWER)
