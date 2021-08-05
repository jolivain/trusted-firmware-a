#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-driver-v2m-flash INTERFACE)

target_sources(tfa-driver-v2m-flash INTERFACE "v2m_flash.c")
