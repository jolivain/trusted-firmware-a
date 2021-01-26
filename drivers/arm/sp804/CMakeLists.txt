#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(sp804-driver-delay-timer INTERFACE)

target_sources(sp804-driver-delay-timer INTERFACE "sp804_delay_timer.c")
target_link_libraries(sp804-driver-delay-timer INTERFACE driver-delay-timer)
