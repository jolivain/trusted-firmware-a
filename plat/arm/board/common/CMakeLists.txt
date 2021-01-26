#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Load in common Arm targets.
#

add_subdirectory("../../common" "${CMAKE_CURRENT_BINARY_DIR}/../../common" EXCLUDE_FROM_ALL)

#
# Define the Arm board target, which is common to all Arm platforms.
#

add_library(platform-arm-board-common INTERFACE)

add_library(platform-arm-board-bl1 INTERFACE)

target_link_libraries(platform-arm-board-bl1
    INTERFACE
        platform-arm-bl1
        platform-arm-board-common)
