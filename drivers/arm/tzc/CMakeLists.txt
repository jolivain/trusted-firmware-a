#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-driver-tzc400 INTERFACE)

target_sources(tfa-driver-tzc400 INTERFACE "tzc400.c")
