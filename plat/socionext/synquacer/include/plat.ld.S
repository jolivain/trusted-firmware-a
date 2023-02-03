/*
 * Copyright (c) 2018-2013, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_PLAT_LD_S__
#define SYNQUACER_PLAT_LD_S__

#include <lib/xlat_tables/xlat_tables_defs.h>

PHDRS {
    sp_dram_x PT_LOAD FLAGS(5 /* R_E */);
    sp_dram_n PT_LOAD FLAGS(6 /* RW_ */);
}

MEMORY {
    sp_dram (rw): ORIGIN = PLAT_SQ_SP_PRIV_BASE, LENGTH = PLAT_SQ_SP_PRIV_SIZE
}

SECTIONS {
    /*
     * Put the page tables in secure DRAM so that the PTW can make cacheable
     * accesses, as the core SPM code expects. (The SRAM on SynQuacer does not
	 * support inner shareable WBWA mappings so it is mapped normal
     * non-cacheable).
     */
    .sp_dram.noinit.sp_xlat_table (NOLOAD) : ALIGN(CONSTANT(MAXPAGESIZE)) {
        *(.sp_xlat_table)
    } >sp_dram AT>sp_dram :sp_dram_n
}

INSERT BEFORE .text.start;

#endif /* SYNQUACER_PLAT_LD_S__ */
