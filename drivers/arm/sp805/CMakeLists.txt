#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(driver-sp805 INTERFACE)

target_sources(driver-sp805 INTERFACE "sp805.c")
