#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-driver-sp804-delay-timer INTERFACE)

target_sources(tfa-driver-sp804-delay-timer INTERFACE "sp804_delay_timer.c")
target_link_libraries(tfa-driver-sp804-delay-timer INTERFACE tfa-driver-delay-timer)
