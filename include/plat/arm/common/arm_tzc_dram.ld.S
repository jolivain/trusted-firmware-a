/*
 * Copyright (c) 2017-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_TZC_DRAM_LD_S
#define ARM_TZC_DRAM_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>

PHDRS {
    el3_tzc_dram_w PT_LOAD FLAGS(6 /* RW_ */);
}

MEMORY {
    el3_tzc_dram (w!a): ORIGIN = ARM_EL3_TZC_DRAM1_BASE, LENGTH = ARM_EL3_TZC_DRAM1_SIZE
}

SECTIONS {
    ASSERT(ARM_EL3_TZC_DRAM1_BASE ==
            ALIGN(ARM_EL3_TZC_DRAM1_BASE, CONSTANT(MAXPAGESIZE)),
        "EL3 secure DRAM address is not aligned on a page boundary")

    .el3_tzc_dram (NOLOAD) : ALIGN(CONSTANT(MAXPAGESIZE)) {
        __EL3_SEC_DRAM_START__ = .;
        *(.arm_el3_tzc_dram)
        __EL3_SEC_DRAM_UNALIGNED_END__ = .;

        . = ALIGN(PAGE_SIZE);

        __EL3_SEC_DRAM_END__ = .;
    } >el3_tzc_dram AT>el3_tzc_dram :el3_tzc_dram_w
}

INSERT BEFORE .text.start;

#endif /* ARM_TZC_DRAM_LD_S */
