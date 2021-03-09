#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(TFA_ENABLE_BACKTRACE)
    add_subdirectory("backtrace" EXCLUDE_FROM_ALL)
endif()

add_library(common-bl INTERFACE)

target_sources(common-bl
    INTERFACE
        "bl_common.c"
        "tf_log.c"
        "${TFA_ARCH}/debug.S")

target_link_libraries(common-bl
    INTERFACE
        driver-multi-console)

add_library(common-fdt-wrappers INTERFACE)

target_sources(common-fdt-wrappers
    INTERFACE
        "fdt_wrappers.c")

target_link_libraries(common-fdt-wrappers
    INTERFACE
        lib-fdt)

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_BACKTRACE

   Enables backtraced crash dumps. This is supported for both AArch64 and
   AArch32, but in AArch32 the format of the frame records is not defined in the
   AAPCS and is instead defined by the implementation. This implementation of
   backtracing only supports the format used by GCC when T32 interworking is
   disabled. For this reason, enabling this option in AArch32 will force the
   compiler to only generate A32 code.

   Enabled by default unless building for AArch32.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_BACKTRACE
    HELP "Enable backtracing in crash dumps."
    DEFAULT ${TFA_AARCH32})

target_compile_definitions(common
    INTERFACE
        "ENABLE_BACKTRACE=$<BOOL:${TFA_ENABLE_BACKTRACE}>")

if(TFA_ENABLE_BACKTRACE)
    target_link_libraries(common
        INTERFACE
            common-backtrace)
endif()
