#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Define the Arm board target, which is common to all Arm platforms.
#

add_library(arm-board-common INTERFACE)

target_sources(arm-board-common
    INTERFACE
        "${TFA_ARCH}/board_arm_helpers.S")

add_library(arm-board-bl1 INTERFACE)

target_link_libraries(arm-board-bl1
    INTERFACE
        arm-bl1
        arm-board-common)
