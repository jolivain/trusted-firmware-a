/*
 * Copyright (c) 2013-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

ENTRY(tsp_entrypoint)

MEMORY {
    ram (rwx): ORIGIN = TSP_SEC_MEM_BASE, LENGTH = TSP_SEC_MEM_SIZE
}

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

SECTIONS {

    .text.entrypoint : ALIGN(CONSTANT(MAXPAGESIZE)) {
        *tsp_entrypoint.o(.text*)
    } >x AT>x :x
}

INSERT BEFORE .text.start

#if USE_COHERENT_MEM
#   define COHERENT_MEMORY_REGION n
#   define COHERENT_MEMORY_PHDR n

#   include <bootloader/coherent_memory.ld.S>

INSERT BEFORE .noinit.end;
#endif /* USE_COHERENT_MEM */
