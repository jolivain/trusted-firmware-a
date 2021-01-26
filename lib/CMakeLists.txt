#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
.. cmake:variable:: TFA_XLAT_TABLES_LIB_V1

   Compile TF-A with version 1 of the translation tables library instead of
   version 2.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_XLAT_TABLES_LIB_V1
    HELP "Compile with version 1 of the translation tables library.")

#
# Create the architecture library. This changes quite a lot based on the
# architecture and the compiler chosen, but there's enough commonality that we
# can lump them into one target.
#

add_library(arch INTERFACE)

target_include_directories(arch
    INTERFACE
        "${PROJECT_SOURCE_DIR}/include/arch/${TFA_ARCH}")

target_sources(arch
    INTERFACE
        "${TFA_ARCH}/cache_helpers.S"
        "${TFA_ARCH}/misc_helpers.S")

if(CMAKE_C_COMPILER_ID STREQUAL "ArmClang")
    target_sources(arch
        INTERFACE
            "${TFA_ARCH}/armclang_printf.S")
endif()

if(TFA_AARCH32)
    target_sources(arch
        INTERFACE
            "aarch32/arm32_aeabi_divmod.c"
            "aarch32/arm32_aeabi_divmod_a32.S")
endif()

#
# Pull in the remaining libraries, which maintain a bit more of a consistent
# directory structure than the architecture library.
#

add_subdirectory("cpus" EXCLUDE_FROM_ALL)
add_subdirectory("el3_runtime" EXCLUDE_FROM_ALL)
add_subdirectory("extensions" EXCLUDE_FROM_ALL)
add_subdirectory("fconf" EXCLUDE_FROM_ALL)
add_subdirectory("libc" EXCLUDE_FROM_ALL)
add_subdirectory("libfdt" EXCLUDE_FROM_ALL)
add_subdirectory("semihosting" EXCLUDE_FROM_ALL)
add_subdirectory("utils" EXCLUDE_FROM_ALL)

if(TFA_XLAT_TABLES_LIB_V1)
    add_subdirectory("xlat_tables" EXCLUDE_FROM_ALL)
else()
    add_subdirectory("xlat_tables_v2" EXCLUDE_FROM_ALL)
endif()
