/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>

ENTRY(bl2_entrypoint)

MEMORY {
#if BL2_IN_XIP_MEM
    rom (rx): ORIGIN = BL2_RO_BASE, LENGTH = BL2_RO_LIMIT - BL2_RO_BASE
    ram (w): ORIGIN = BL2_RW_BASE, LENGTH = BL2_RW_LIMIT - BL2_RW_BASE
#else /* BL2_IN_XIP_MEM */
    ram (rwx): ORIGIN = BL2_BASE, LENGTH = BL2_LIMIT - BL2_BASE
#endif /* BL2_IN_XIP_MEM */

#if SEPARATE_BL2_NOLOAD_REGION
    nobits (w!a): ORIGIN = BL2_NOLOAD_START, LENGTH = BL2_NOLOAD_LIMIT - BL2_NOLOAD_START
#endif /* SEPARATE_BL2_NOLOAD_REGION */
}

REGION_ALIAS("w", ram)

#if BL2_IN_XIP_MEM
REGION_ALIAS("x", rom)
REGION_ALIAS("r", rom)
#else /* BL2_IN_XIP_MEM */
REGION_ALIAS("x", ram)
REGION_ALIAS("r", ram)
#endif /* BL2_IN_XIP_MEM */

#if SEPARATE_BL2_NOLOAD_REGION
REGION_ALIAS("n", nobits)
#else /* SEPARATE_BL2_NOLOAD_REGION */
REGION_ALIAS("n", ram)
#endif /* SEPARATE_BL2_NOLOAD_REGION */

#if BL2_IN_XIP_MEM
ASSERT(BL2_RO_BASE == ALIGN(BL2_RO_BASE, CONSTANT(MAXPAGESIZE)),
    "BL2 ROM address not aligned on a page boundary")
ASSERT(BL2_RW_BASE == ALIGN(BL2_RW_BASE, CONSTANT(MAXPAGESIZE)),
    "BL2 RAM address not aligned on a page boundary")
#else /* BL2_IN_XIP_MEM */
ASSERT(BL2_BASE == ALIGN(BL2_BASE, CONSTANT(MAXPAGESIZE)),
    "BL2 address not aligned on a page boundary")
#endif /* BL2_IN_XIP_MEM */

#if SEPARATE_BL2_NOLOAD_REGION
ASSERT(BL2_NOLOAD_START == ALIGN(BL2_NOLOAD_START, CONSTANT(MAXPAGESIZE)),
    "BL2 NOLOAD address not aligned on a page boundary")
#endif /* SEPARATE_BL2_NOLOAD_REGION */

SECTIONS {
    __RAM_REGION_LENGTH__ = LENGTH(ram);
    __RAM_REGION_START__ = ORIGIN(ram);
    __RAM_REGION_END__ = __N_END__;

#if SEPARATE_BL2_NOLOAD_REGION
    __RAM_REGION_END__ = __W_END__;
#endif /* SEPARATE_BL2_NOLOAD_REGION */

#if BL2_IN_XIP_MEM
    __ROM_REGION_LENGTH__ = LENGTH(rom);
    __ROM_REGION_START__ = ORIGIN(rom);
    __ROM_REGION_END__ = __R_END__;
#endif /* BL2_IN_XIP_MEM */

#if SEPARATE_BL2_NOLOAD_REGION
    __NOBITS_REGION_LENGTH__ = LENGTH(nobits);
    __NOBITS_REGION_START__ = ORIGIN(nobits);
    __NOBITS_REGION_END__ = __N_END__;
#endif /* SEPARATE_BL2_NOLOAD_REGION */

#if BL2_IN_XIP_MEM
    __BL2_ROM_START__ = __ROM_REGION_START__;
    __BL2_ROM_END__ = __ROM_REGION_END__;

    __BL2_RAM_START__ = __RAM_REGION_START__;
    __BL2_RAM_END__ = __RAM_REGION_END__;
#else /* BL2_IN_XIP_MEM */
    __BL2_START__ = __RAM_REGION_START__;
    __BL2_END__ = __RAM_REGION_END__;
#endif /* BL2_IN_XIP_MEM */

#if SEPARATE_BL2_NOLOAD_REGION
    __BL2_NOLOAD_START__ = __NOBITS_REGION_START__;
    __BL2_NOLOAD_END__ = __NOBITS_REGION_END__;
#endif /* SEPARATE_BL2_NOLOAD_REGION */

    __RO_START__ = __X_START__;
    __RO_END__ = __R_END__;

    __RW_START__ = __W_START__;
    __RW_END__ = __N_END__;

#if SEPARATE_BL2_NOLOAD_REGION
    __RW_END__ = __W_END__;
#endif /* SEPARATE_BL2_NOLOAD_REGION */

    HIDDEN(__RODATA_PADDING__ = CONSTANT(MAXPAGESIZE)); /* TODO: Why? */
}

INSERT BEFORE .text.start;

SECTIONS {
    __TEXT_RESIDENT_START__ = .;
}

INSERT BEFORE .text.entrypoint;

SECTIONS {
    .text.resident : {
        *(.text.asm.*) /* TODO: Can we be more specific than this? */
    }

    ASSERT((__TEXT_RESIDENT_END__ - __TEXT_RESIDENT_START__) <= CONSTANT(MAXPAGESIZE),
        "resident executable data has exceeded its limit")

    __TEXT_RESIDENT_END__ = .;
}

INSERT AFTER .text.entrypoint;
