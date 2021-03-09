#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(driver-delay-timer INTERFACE)

target_sources(driver-delay-timer
    INTERFACE
        "delay_timer.c")

add_library(driver-generic-delay-timer INTERFACE)

target_sources(driver-generic-delay-timer
    INTERFACE
        "generic_delay_timer.c")

target_link_libraries(driver-generic-delay-timer
    INTERFACE
        driver-delay-timer)
