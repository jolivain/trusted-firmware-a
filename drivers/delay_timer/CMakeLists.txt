#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-driver-delay-timer INTERFACE)

target_sources(tfa-driver-delay-timer
    INTERFACE "delay_timer.c")

add_library(tfa-driver-generic-delay-timer INTERFACE)

target_sources(tfa-driver-generic-delay-timer
    INTERFACE "generic_delay_timer.c")

target_link_libraries(tfa-driver-generic-delay-timer
    INTERFACE tfa-driver-delay-timer)
