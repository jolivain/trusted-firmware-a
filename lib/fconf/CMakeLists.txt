#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(lib-fconf INTERFACE)

target_sources(lib-fconf
    INTERFACE
        "fconf.c")

target_link_libraries(lib-fconf
    INTERFACE
        common-fdt-wrappers)

add_library(lib-fconf-dynamic INTERFACE)

target_sources(lib-fconf-dynamic
    INTERFACE
        "fconf_dyn_cfg_getter.c")

target_link_libraries(lib-fconf-dynamic
    INTERFACE
        common-fdt-wrappers)
