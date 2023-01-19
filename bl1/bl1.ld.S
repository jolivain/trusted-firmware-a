/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>

ENTRY(bl1_entrypoint)

MEMORY {
    rom (rx): ORIGIN = BL1_RO_BASE, LENGTH = BL1_RO_LIMIT - BL1_RO_BASE
    ram (w): ORIGIN = BL1_RW_BASE, LENGTH = BL1_RW_LIMIT - BL1_RW_BASE
}

REGION_ALIAS("x", rom)
REGION_ALIAS("r", rom)
REGION_ALIAS("w", ram)
REGION_ALIAS("n", ram)

ASSERT(BL1_RO_BASE == ALIGN(BL1_RO_BASE, CONSTANT(MAXPAGESIZE)),
    "BL1 ROM address not aligned on a page boundary")

ASSERT(BL1_RW_BASE == ALIGN(BL1_RW_BASE, CONSTANT(MAXPAGESIZE)),
    "BL1 RAM address not aligned on a page boundary")

SECTIONS {
    __ROM_REGION_START__ = __R_REGION_START__;
    __ROM_REGION_END__ = __R_REGION_END__;
    __ROM_REGION_LENGTH__ = __R_REGION_LENGTH__;

    __RAM_REGION_START__ = __W_REGION_START__;
    __RAM_REGION_END__ = __W_REGION_END__;
    __RAM_REGION_LENGTH__ = __W_REGION_LENGTH__;

    __BL1_ROM_START__ = __ROM_REGION_START__;
    __BL1_ROM_END__ = __ROM_REGION_END__;

    __BL1_RAM_START__ = __RAM_REGION_START__;
    __BL1_RAM_END__ = __RAM_REGION_END__;

    __DATA_RAM_START__ = __DATA_START__;
    __DATA_RAM_END__ = __DATA_END__;

    __DATA_ROM_START__ = __DATA_LOAD_START__;
    __DATA_ROM_END__ = __DATA_LOAD_END__;

    HIDDEN(__DATA_ALIGNMENT__ = 16); /* Required by the C runtime */
    HIDDEN(__DATA_PADDING__ = 16); /* Required by the C runtime */

#if !SEPARATE_CODE_AND_RODATA
    __RO_START__ = __TEXT_START__;
    __RO_END_UNALIGNED__ = ADDR(.rodata.end);
    __RO_END__ = __RODATA_END__;
#endif /* SEPARATE_CODE_AND_RODATA */
}

INSERT BEFORE .text.start;

#if USE_COHERENT_MEM
#   define COHERENT_MEMORY_REGION n
#   define COHERENT_MEMORY_PHDR n

#   include <bootloader/coherent_memory.ld.S>

INSERT BEFORE .noinit.end;
#endif /* USE_COHERENT_MEM */
