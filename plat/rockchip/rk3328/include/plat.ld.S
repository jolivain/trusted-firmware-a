/*
 * Copyright (c) 2016-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ROCKCHIP_PLAT_LD_S
#define ROCKCHIP_PLAT_LD_S

PHDRS {
    pmusram_xr PT_LOAD FLAGS(5 /* R_E */);
    pmusram_w PT_LOAD FLAGS(6 /* RW_ */);
}

MEMORY {
    pmusram (rwx): ORIGIN = PMUSRAM_BASE, LENGTH = PMUSRAM_RSIZE
}

SECTIONS {
    /*
     * pmu_cpuson_entrypoint request address align 64K when resume, so put it in
     * the start of pmusram
     */
    .pmusram.text : {
        ASSERT(. == ALIGN(64 * 1024), ".pmusram.entry request 64K aligned.");

        *(.pmusram.entry)

        __bl31_pmusram_text_start = .;
        *(.pmusram.text)
        *(.pmusram.rodata)
        __bl31_pmusram_text_end = .;
    } >pmusram AT>pmusram :pmusram_xr

    .pmusram.data : {
        __bl31_pmusram_data_start = .;
        *(.pmusram.data)
        __bl31_pmusram_data_end = .;
    } >pmusram AT>pmusram :pmusram_w
}

INSERT BEFORE .text.start;

SECTIONS {
    .text.sram.text : {
        *(.sram.text)
    } >x AT>x :x
}

INSERT AFTER .text;

SECTIONS {
    .rodata.sram.rodata : {
        *(.sram.rodata)
    } >r AT>r :r
}

INSERT AFTER .rodata;

SECTIONS {
    .data.sram.data : {
        *(.sram.data)
    } >w AT>r :w
}

INSERT AFTER .data;

#endif /* ROCKCHIP_PLAT_LD_S */
