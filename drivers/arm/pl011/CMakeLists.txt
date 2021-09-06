#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-driver-arm-pl011-console INTERFACE)

target_sources(tfa-driver-arm-pl011-console
    INTERFACE "${TFA_ARCH_STATE_LOWER}/pl011_console.S")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PL011_GENERIC_UART

   Indicates that to the PL011 driver that the underlying hardware is not a
   fully-compliant PL011 UART but a minimally compliant generic UART, which is a
   subset of the PL011. The driver will not access any register that is not part
   of the SBSA generic UART specification.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_PL011_GENERIC_UART
    HELP "Restrict the PL011 driver to the SBSA generic UART specification?")

target_compile_definitions(tfa-driver-arm-pl011-console
    INTERFACE "PL011_GENERIC_UART=$<BOOL:${TFA_PL011_GENERIC_UART}>")
