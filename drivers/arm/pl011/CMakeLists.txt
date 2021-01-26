#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(driver-arm-pl011-console INTERFACE)

target_sources(driver-arm-pl011-console
    INTERFACE
        "${TFA_ARCH}/pl011_console.S")
