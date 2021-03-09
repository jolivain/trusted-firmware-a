#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(common-backtrace INTERFACE)

target_compile_options(common-backtrace
    INTERFACE
        "-fno-omit-frame-pointer")

target_sources(common-backtrace
    INTERFACE
        "backtrace.c")
