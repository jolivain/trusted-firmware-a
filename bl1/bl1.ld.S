/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>

ENTRY(bl1_entrypoint)

MEMORY {
    r (rx): ORIGIN = BL1_RO_BASE, LENGTH = BL1_RO_LIMIT - BL1_RO_BASE
    w (w): ORIGIN = BL1_RW_BASE, LENGTH = BL1_RW_LIMIT - BL1_RW_BASE
}

REGION_ALIAS("x", r)
REGION_ALIAS("n", w)

ASSERT(BL1_RO_BASE == ALIGN(BL1_RO_BASE, CONSTANT(MAXPAGESIZE)),
    "BL1 ROM address not aligned on a page boundary")

ASSERT(BL1_RW_BASE == ALIGN(BL1_RW_BASE, CONSTANT(MAXPAGESIZE)),
    "BL1 RAM address not aligned on a page boundary")

SECTIONS {
    __BL1_ROM_START__ = __X_START__;
    __BL1_ROM_END__ = __R_END__;

    __BL1_RAM_START__ = __W_START__;
    __BL1_RAM_END__ = __N_END__;

    __DATA_RAM_START__ = __DATA_START__; /* For compatibility */
    __DATA_RAM_END__ = __DATA_END__; /* For compatibility */

    __DATA_ROM_START__ = __DATA_LOAD_START__; /* For compatibility */
    __DATA_ROM_END__ = __DATA_LOAD_END__; /* For compatibility */
}

INSERT BEFORE .text.entrypoint;
