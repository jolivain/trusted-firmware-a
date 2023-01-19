/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Switch out the memory region for the base translation table depending on
 * whether it's expected to be made read-only or not.
 */
#if PLAT_RO_XLAT_TABLES
#   define BASE_XLAT_TABLE_NAME .rodata.base_xlat_table
#   define BASE_XLAT_TABLE_NOLOAD
#   define BASE_XLAT_TABLE_REGION r
#   define BASE_XLAT_TABLE_PHDR r
#else
#   define BASE_XLAT_TABLE_NAME .bss.base_xlat_table
#   define BASE_XLAT_TABLE_NOLOAD (NOLOAD)
#   define BASE_XLAT_TABLE_REGION n
#   define BASE_XLAT_TABLE_PHDR n
#endif /* PLAT_RO_XLAT_TABLES */

SECTIONS {
    BASE_XLAT_TABLE_NAME BASE_XLAT_TABLE_NOLOAD : {
        __BASE_XLAT_TABLE_START__ = .;
        KEEP(*(.base_xlat_table))
        __BASE_XLAT_TABLE_END__ = .;
    } >BASE_XLAT_TABLE_REGION AT>BASE_XLAT_TABLE_REGION :BASE_XLAT_TABLE_PHDR
}
