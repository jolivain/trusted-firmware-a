#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(fconf INTERFACE)

target_sources(fconf
    INTERFACE
        "fconf.c")

target_link_libraries(fconf
    INTERFACE
        fdt-wrappers)

add_library(fconf-dynamic INTERFACE)

target_sources(fconf-dynamic
    INTERFACE
        "fconf_dyn_cfg_getter.c")

target_link_libraries(fconf-dynamic
    INTERFACE
        fdt-wrappers)
