#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-utils INTERFACE)

target_sources(tfa-utils INTERFACE "mem_region.c")
target_link_libraries(tfa-utils INTERFACE tfa-common)
