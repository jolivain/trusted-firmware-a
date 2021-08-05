#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-lib-xlat-tables INTERFACE)

target_compile_definitions(tfa-lib-xlat-tables
    INTERFACE "XLAT_TABLES_LIB_V2=1")

target_sources(tfa-lib-xlat-tables
    INTERFACE "xlat_tables_context.c"
              "xlat_tables_core.c"
              "xlat_tables_utils.c"
              "${TFA_ARCH_STATE_LOWER}/enable_mmu.S"
              "${TFA_ARCH_STATE_LOWER}/xlat_tables_arch.c")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ALLOW_RO_XLAT_TABLES

   Dictates whether the translation tables can be placed in the read-only
   section of the bootloader binaries (``.rodata``). If not, they are always
   placed in the zero-initialized section (``.bss``).

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ALLOW_RO_XLAT_TABLES
    HELP "Allow translation tables to be placed in the read-only data section.")

target_compile_definitions(tfa-lib-xlat-tables
    INTERFACE "ALLOW_RO_XLAT_TABLES=$<BOOL:${ALLOW_RO_XLAT_TABLES}>")
