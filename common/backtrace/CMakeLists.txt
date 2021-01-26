#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(backtrace INTERFACE)

target_compile_options(backtrace
    INTERFACE
        "-fno-omit-frame-pointer")

target_compile_definitions(backtrace
    INTERFACE
        "ENABLE_BACKTRACE=1")

target_sources(backtrace
    INTERFACE
        "backtrace.c")
