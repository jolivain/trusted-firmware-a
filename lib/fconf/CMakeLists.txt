#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-lib-fconf INTERFACE)

target_sources(tfa-lib-fconf
    INTERFACE "fconf.c")

target_link_libraries(tfa-lib-fconf
    INTERFACE tfa-common-fdt-wrappers)

add_library(tfa-lib-fconf-dynamic INTERFACE)

target_sources(tfa-lib-fconf-dynamic
    INTERFACE "fconf_dyn_cfg_getter.c")

target_link_libraries(tfa-lib-fconf-dynamic
    INTERFACE tfa-common-fdt-wrappers)
