#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-lib-fdt INTERFACE)

target_include_directories(tfa-lib-fdt
    INTERFACE "${TFA_SOURCE_DIR}/include/lib/libfdt")

target_sources(tfa-lib-fdt
    INTERFACE "${CMAKE_CURRENT_LIST_DIR}/fdt.c"
              "${CMAKE_CURRENT_LIST_DIR}/fdt_addresses.c"
              "${CMAKE_CURRENT_LIST_DIR}/fdt_empty_tree.c"
              "${CMAKE_CURRENT_LIST_DIR}/fdt_ro.c"
              "${CMAKE_CURRENT_LIST_DIR}/fdt_rw.c"
              "${CMAKE_CURRENT_LIST_DIR}/fdt_strerror.c"
              "${CMAKE_CURRENT_LIST_DIR}/fdt_sw.c"
              "${CMAKE_CURRENT_LIST_DIR}/fdt_wip.c")
