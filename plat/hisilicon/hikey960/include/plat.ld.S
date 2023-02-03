/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_LD_S
#define PLAT_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>

PHDRS {
    ram2_n PT_LOAD FLAGS(6 /* RW_ */);
}

MEMORY {
    ram2 (rw): ORIGIN = DDR2_SEC_BASE, LENGTH = DDR2_SEC_SIZE
}

SECTIONS {
    .ram2 (NOLOAD) : {
        *(.ram2_region)
    } >ram2 AT>ram2 :ram2_n
}

INSERT BEFORE .text.start;

#endif /* PLAT_LD_S */
