#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(utils INTERFACE)

target_sources(utils INTERFACE "mem_region.c")
target_link_libraries(utils INTERFACE common)
