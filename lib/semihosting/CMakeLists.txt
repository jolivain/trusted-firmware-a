#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-lib-semihosting INTERFACE)

target_sources(tfa-lib-semihosting INTERFACE "semihosting.c"
                                   "${TFA_ARCH_STATE_LOWER}/semihosting_call.S")

target_link_libraries(tfa-lib-semihosting INTERFACE tfa-common)
