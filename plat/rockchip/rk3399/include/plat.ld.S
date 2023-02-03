/*
 * Copyright (c) 2016-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ROCKCHIP_PLAT_LD_S
#define ROCKCHIP_PLAT_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>

PHDRS {
    sram_xr PT_LOAD FLAGS(5 /* R_E */);
    sram_w PT_LOAD FLAGS(6 /* RW_ */);
    sram_n PT_LOAD FLAGS(6 /* RW_ */);

    pmusram_xr PT_LOAD FLAGS(5 /* R_E */);
    pmusram_w PT_LOAD FLAGS(6 /* RW_ */);
}

MEMORY {
    sram (rwx): ORIGIN = SRAM_BASE, LENGTH = SRAM_SIZE
    pmusram (rwx): ORIGIN = PMUSRAM_BASE, LENGTH = PMUSRAM_RSIZE
}

SECTIONS {
    ASSERT(SRAM_BASE == ALIGN(SRAM_BASE, CONSTANT(MAXPAGESIZE)),
        "SRAM_BASE address is not aligned on a page boundary.")

    /*
     * The SRAM space allocation for RK3399
     * ----------------
     * | m0 code bin
     * ----------------
     * | sram text
     * ----------------
     * | sram data
     * ----------------
     */
    .sram.incbin : ALIGN(CONSTANT(MAXPAGESIZE)) {
        __sram_incbin_start = .;
        *(.sram.incbin)
        __sram_incbin_real_end = .;

        . = ALIGN(CONSTANT(MAXPAGESIZE));

        __sram_incbin_end = .;
    } >sram AT>sram :sram_xr

    ASSERT((__sram_incbin_real_end - __sram_incbin_start) <=
        SRAM_BIN_LIMIT, ".incbin_sram has exceeded its limit")

    .sram.text : ALIGN(CONSTANT(MAXPAGESIZE)) {
        __bl31_sram_text_start = .;
        *(.sram.text)
        *(.sram.rodata)
        __bl31_sram_text_real_end = .;

        . = ALIGN(CONSTANT(MAXPAGESIZE));

        __bl31_sram_text_end = .;
    } >sram AT>sram :sram_xr

    ASSERT((__bl31_sram_text_real_end - __bl31_sram_text_start) <=
        SRAM_TEXT_LIMIT, ".text_sram has exceeded its limit")

    .sram.data : ALIGN(CONSTANT(MAXPAGESIZE)) {
        __bl31_sram_data_start = .;
        *(.sram.data)
        __bl31_sram_data_real_end = .;

        . = ALIGN(CONSTANT(MAXPAGESIZE));

        __bl31_sram_data_end = .;
    } >sram AT>sram :sram_w

    ASSERT((__bl31_sram_data_real_end - __bl31_sram_data_start) <=
        SRAM_DATA_LIMIT, ".data_sram has exceeded its limit")

    .sram.stack (NOLOAD) : ALIGN(CONSTANT(MAXPAGESIZE)) {
        __bl31_sram_stack_start = .;
        . += CONSTANT(MAXPAGESIZE);
        __bl31_sram_stack_end = .;
    } >sram AT>sram :sram_n

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

    /* M0 start address request 4K align */
    .pmsusram.incbin : ALIGN(4096) {
        __pmusram_incbin_start = .;
        *(.pmusram.incbin)
        __pmusram_incbin_end = .;
    } >pmusram AT>pmusram :pmusram_w

    .pmusram.data : {
        __bl31_pmusram_data_start = .;
        *(.pmusram.data)
        __bl31_pmusram_data_end = .;
    } >pmusram AT>pmusram :pmusram_w
}

INSERT BEFORE .text.start;

#endif /* ROCKCHIP_PLAT_LD_S */
