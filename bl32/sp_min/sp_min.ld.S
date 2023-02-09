/*
 * Copyright (c) 2016-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

ENTRY(sp_min_vector_table)

MEMORY {
    ram (rwx): ORIGIN = BL32_BASE, LENGTH = BL32_LIMIT - BL32_BASE
}

#ifdef PLAT_SP_MIN_EXTRA_LD_SCRIPT
#   include <plat_sp_min.ld.S>
#endif /* PLAT_SP_MIN_EXTRA_LD_SCRIPT */

REGION_ALIAS("r", ram)
REGION_ALIAS("w", ram)
REGION_ALIAS("x", ram)
REGION_ALIAS("n", ram)

SECTIONS {
    __RAM_REGION_LENGTH__ = LENGTH(ram);
    __RAM_REGION_START__ = ORIGIN(ram);
    __RAM_REGION_END__ = __N_REGION_END__;

    __RW_START__ = __N_REGION_START__;
    __RW_END__ = __N_REGION_END__;

    __RO_START__ = __R_REGION_START__;
    __RO_END__ = __R_REGION_END__;

    __BL32_START__ = __RAM_REGION_START__;
    __BL32_END__ = __RAM_REGION_END__;
}

INSERT BEFORE .text.start

SECTIONS {

    .text.entrypoint : ALIGN(CONSTANT(MAXPAGESIZE)) {
        *entrypoint.o(.text*)
    } >x AT>x :x
}

INSERT AFTER .text.start;

SECTIONS {
    /*
     * Pub-sub events.
     */
    .rodata.pubsub_events : {
#include <lib/el3_runtime/pubsub_events.h>
    } >r AT>r :r
}

INSERT AFTER .rodata;

#if USE_COHERENT_MEM
#   define COHERENT_MEMORY_REGION n
#   define COHERENT_MEMORY_PHDR n

#   include <bootloader/coherent_memory.ld.S>

INSERT BEFORE .noinit.end;
#endif /* USE_COHERENT_MEM */

#if PLAT_EXTRA_RODATA_INCLUDES
#   include <plat.ld.rodata.inc>
#endif /* PLAT_EXTRA_RODATA_INCLUDES */
