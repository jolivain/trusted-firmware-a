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
    __DATA_ALIGNMENT__ = 16; /* Required by the C runtime */
    __DATA_PADDING__ = 16; /* Required by the C runtime */

    __ROM_REGION_LENGTH__ = LENGTH(rom); /* For metrics collection */
    __ROM_REGION_START__ = ORIGIN(rom); /* For metrics collection */
    __ROM_REGION_END__ = __R_END__; /* For metrics collection */

    __RAM_REGION_START__ = ORIGIN(ram); /* For metrics collection */
    __RAM_REGION_END__ = __N_END__; /* For metrics collection */
    __RAM_REGION_LENGTH__ = LENGTH(ram); /* For metrics collection */

    __BL1_ROM_START__ = __ROM_REGION_START__; /* For backwards compatibility */
    __BL1_ROM_END__ = __ROM_REGION_END__; /* For backwards compatibility */

    __BL1_RAM_START__ = __RAM_REGION_START__; /* For backwards compatibility */
    __BL1_RAM_END__ = __RAM_REGION_END__; /* For backwards compatibility */

    __RO_START__ = __X_START__; /* For backwards compatibility */
    __RO_END__ = __RODATA_END__; /* For backwards compatibility */

    __DATA_RAM_START__ = __DATA_START__; /* For backwards compatibility */
    __DATA_RAM_END__ = __DATA_END__; /* For backwards compatibility */

    __DATA_ROM_START__ = __DATA_LOAD_START__; /* For backwards compatibility */
    __DATA_ROM_END__ = __DATA_LOAD_END__; /* For backwards compatibility */
}

INSERT BEFORE .text.start;
