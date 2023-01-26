/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>

ENTRY(bl2_entrypoint)

MEMORY {
    ram (rwx): ORIGIN = BL2_BASE, LENGTH = BL2_LIMIT - BL2_BASE
}

REGION_ALIAS("x", ram)
REGION_ALIAS("r", ram)
REGION_ALIAS("w", ram)
REGION_ALIAS("n", ram)

ASSERT(BL2_BASE == ALIGN(BL2_BASE, CONSTANT(MAXPAGESIZE)),
    "BL2 address not aligned on a page boundary")

SECTIONS {
    __RAM_REGION_START__ = __X_REGION_START__;
    __RAM_REGION_END__ = __X_REGION_END__;
    __RAM_REGION_LENGTH__ = __X_REGION_LENGTH__;

    __BL2_START__ = __RAM_REGION_START__;
    __BL2_END__ = __RAM_REGION_END__;

#if !SEPARATE_CODE_AND_RODATA
    __RO_START__ = __TEXT_START__;
    __RO_END_UNALIGNED__ = ADDR(.rodata.end);
    __RO_END__ = __RODATA_END__;
#endif /* SEPARATE_CODE_AND_RODATA */

    __RW_START__ = __DATA_START__;
    __RW_END__ = __NOINIT_END__;
}

INSERT BEFORE .text.start;

#if USE_COHERENT_MEM
#   define COHERENT_MEMORY_REGION n
#   define COHERENT_MEMORY_PHDR n

#   include <bootloader/coherent_memory.ld.S>

INSERT BEFORE .noinit.end;
#endif /* USE_COHERENT_MEM */
