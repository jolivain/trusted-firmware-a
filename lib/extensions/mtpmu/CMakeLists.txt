#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-lib-extension-mtpmu INTERFACE)

target_sources(tfa-lib-extension-mtpmu
    INTERFACE "${TFA_ARCH_STATE_LOWER}/mtpmu.S")
