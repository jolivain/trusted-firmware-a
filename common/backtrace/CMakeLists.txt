#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-common-backtrace INTERFACE)

target_compile_options(tfa-common-backtrace
    INTERFACE "-fno-omit-frame-pointer")

target_sources(tfa-common-backtrace
    INTERFACE "backtrace.c")
