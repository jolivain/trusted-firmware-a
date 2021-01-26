#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(driver-multi-console INTERFACE)

target_sources(driver-multi-console INTERFACE "multi_console.c")
