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

add_library(fdt-wrappers INTERFACE)

target_sources(fdt-wrappers
    INTERFACE
        "fdt_wrappers.c")

target_link_libraries(fdt-wrappers
    INTERFACE
        fdt)
