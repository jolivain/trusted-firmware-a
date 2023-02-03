/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>

ENTRY(bl31_entrypoint)

MEMORY {
    ram (rwx): ORIGIN = BL31_BASE, LENGTH = BL31_LIMIT - BL31_BASE

#if SEPARATE_NOBITS_REGION
    nobits (w!a): ORIGIN = BL31_NOBITS_BASE, LENGTH = BL31_NOBITS_LIMIT - BL31_NOBITS_BASE
#endif /* SEPARATE_NOBITS_REGION */
}

REGION_ALIAS("x", ram)
REGION_ALIAS("r", ram)
REGION_ALIAS("w", ram)

#if SEPARATE_NOBITS_REGION
REGION_ALIAS("n", nobits)
#else /* SEPARATE_NOBITS_REGION */
REGION_ALIAS("n", ram)
#endif /* SEPARATE_NOBITS_REGION */

ASSERT(BL31_BASE == ALIGN(BL31_BASE, CONSTANT(MAXPAGESIZE)),
    "BL31 address not aligned on a page boundary")

#if SEPARATE_NOBITS_REGION
ASSERT(BL31_NOBITS_BASE == ALIGN(BL31_NOBITS_BASE, CONSTANT(MAXPAGESIZE)),
    "BL31 NOBITS address not aligned on a page boundary")
#endif /* SEPARATE_NOBITS_REGION */

SECTIONS {
    __RAM_REGION_START__ = __W_REGION_START__;
    __RAM_REGION_END__ = __W_REGION_END__;
    __RAM_REGION_LENGTH__ = __W_REGION_LENGTH__;

#if SEPARATE_NOBITS_REGION
    __NOBITS_REGION_START__ = __N_REGION_START__;
    __NOBITS_REGION_END__ = __N_REGION_END__;
    __NOBITS_REGION_LENGTH__ = __N_REGION_LENGTH__;

    __NOBITS_START__ = __NOBITS_REGION_START__;
    __NOBITS_END__ = __NOBITS_REGION_END__;
#endif /* SEPARATE_NOBITS_REGION */

    __BL31_START__ = __RAM_REGION_START__;
    __BL31_END__ = __RAM_REGION_END__;

#if !SEPARATE_CODE_AND_RODATA
    __RO_START__ = __TEXT_START__;
    __RO_END_UNALIGNED__ = ADDR(.rodata.end);
    __RO_END__ = __RODATA_END__;
#endif /* SEPARATE_CODE_AND_RODATA */

    __RW_START__ = __DATA_START__;
    __RW_END__ = __NOINIT_END__;
}

INSERT BEFORE .text.start;

SECTIONS {
    /*
     * Pub-sub events.
     */
    .rodata.pubsub_events : {
#include <lib/el3_runtime/pubsub_events.h>
    } >r AT>r :r
}

INSERT AFTER .rodata;

#if SPM_MM
#   include <bootloader/spm_shim_exceptions.ld.S>
#endif /* SPM_MM */

#if USE_COHERENT_MEM
#   define COHERENT_MEMORY_REGION n
#   define COHERENT_MEMORY_PHDR n

#   include <bootloader/coherent_memory.ld.S>

INSERT BEFORE .noinit.end;
#endif /* USE_COHERENT_MEM */

#if RECLAIM_INIT_CODE
#   include <bootloader/reclaim_init_code.ld.S>
#endif /*RECLAIM_INIT_CODE */
