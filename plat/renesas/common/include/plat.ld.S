/*
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef RCAR_PLAT_LD_S
#define RCAR_PLAT_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <platform_def.h>

PHDRS {
    pram_w PT_LOAD FLAGS(6 /* RW_ */);
}

MEMORY {
    sram (rw): ORIGIN = BL31_SRAM_BASE, LENGTH = DEVICE_SRAM_SIZE
    pram (rw): ORIGIN = BL31_LIMIT - DEVICE_SRAM_SIZE, LENGTH = DEVICE_SRAM_SIZE
}

SECTIONS {
    /* SRAM_COPY is in PRAM */
    __SRAM_COPY_START__ = BL31_LIMIT - DEVICE_SRAM_SIZE;

    .pram : {
        /* system ram start is in SRAM */
        __system_ram_start__ = .;
        *(.system_ram*)
        *iic_dvfs.o(.rodata)
		__system_ram_end__ = .;
    } >sram AT>pram :pram_w

    ASSERT(__BL31_END__ <= BL31_LIMIT - DEVICE_SRAM_SIZE,
    	"BL31 image too large - writing on top of SRAM!")
}

INSERT BEFORE .text.start;

#endif /* RCAR_PLAT_LD_S */
