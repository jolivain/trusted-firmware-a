#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(driver-cci INTERFACE)

target_sources(driver-cci INTERFACE "cci.c")
