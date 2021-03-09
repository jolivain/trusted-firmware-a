#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(lib-semihosting INTERFACE)

target_sources(lib-semihosting INTERFACE "semihosting.c"
                                   "${TFA_ARCH}/semihosting_call.S")

target_link_libraries(lib-semihosting INTERFACE common)
