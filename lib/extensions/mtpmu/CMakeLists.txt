#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(lib-extension-mtpmu INTERFACE)

target_sources(lib-extension-mtpmu
    INTERFACE
        "${TFA_ARCH}/mtpmu.S")
