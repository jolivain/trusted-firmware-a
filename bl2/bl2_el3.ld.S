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
#if BL2_IN_XIP_MEM
    __ROM_REGION_START__ = __X_REGION_START__;
    __ROM_REGION_END__ = __X_REGION_END__;
    __ROM_REGION_LENGTH__ = __X_REGION_LENGTH__;
#endif /* BL2_IN_XIP_MEM */

    __RAM_REGION_START__ = __W_REGION_START__;
    __RAM_REGION_END__ = __W_REGION_END__;
    __RAM_REGION_LENGTH__ = __W_REGION_LENGTH__;

#if SEPARATE_BL2_NOLOAD_REGION
    __NOBITS_REGION_START__ = __N_REGION_START__;
    __NOBITS_REGION_END__ = __N_REGION_END__;
    __NOBITS_REGION_LENGTH__ = __N_REGION_LENGTH__;
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

#if !SEPARATE_CODE_AND_RODATA
    __RO_START__ = __TEXT_START__;
    __RO_END_UNALIGNED__ = ADDR(.rodata.end);
    __RO_END__ = __RODATA_END__;
#endif /* SEPARATE_CODE_AND_RODATA */

    __RW_START__ = __DATA_START__;
    __RW_END__ = __W_REGION_END__; /* Includes the coherent memory section */

#if USE_COHERENT_MEM && SEPARATE_BL2_NOLOAD_REGION
    __RW_END__ = __COHERENT_RAM_END__;
#endif /* SEPARATE_BL2_NOLOAD_REGION */
}

INSERT BEFORE .text.start;

SECTIONS {
    .text.resident : {
        *(.text.asm.*)
    }

    ASSERT((__TEXT_RESIDENT_END__ - __TEXT_RESIDENT_START__) <= CONSTANT(MAXPAGESIZE),
        "resident executable data has exceeded its limit")

    __TEXT_RESIDENT_START__ = __TEXT_START__;
    __TEXT_RESIDENT_END__ = .;
}

INSERT AFTER .text.entrypoint;

/*
 * Coherent memory in this image, despite being NOBITS, has historically been
 * placed immediately following writable data even when NOBITS data has been
 * given a dedicated memory region.
 *
 * It's not clear if that is intentional because that's not the case with other
 * images, but avoid tempting fate by maintaining the existing behaviour.
 */
#if USE_COHERENT_MEM
#   if SEPARATE_BL2_NOLOAD_REGION
PHDRS {
    c PT_LOAD FLAGS(6 /* RW_ */);
}

#       define COHERENT_MEMORY_REGION w
#       define COHERENT_MEMORY_PHDR c
#   else /* SEPARATE_BL2_NOLOAD_REGION */
#       define COHERENT_MEMORY_REGION n
#       define COHERENT_MEMORY_PHDR n
#   endif /* SEPARATE_BL2_NOLOAD_REGION */

#   include <bootloader/coherent_memory.ld.S>

INSERT AFTER .data.end;
#endif /* USE_COHERENT_MEM */
