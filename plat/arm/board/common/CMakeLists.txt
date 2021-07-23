#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Define the Arm board target, which is common to all Arm platforms.
#

add_library(tfa-arm-board-common INTERFACE)

target_sources(tfa-arm-board-common
    INTERFACE "${TFA_ARCH_STATE_LOWER}/board_arm_helpers.S")

add_library(tfa-arm-board-bl1 INTERFACE)

target_link_libraries(tfa-arm-board-bl1
    INTERFACE tfa-arm-bl1
              tfa-arm-board-common)
