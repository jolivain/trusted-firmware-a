/*
 * Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ROCKCHIP_PLAT_LD_S__
#define __ROCKCHIP_PLAT_LD_S__

PHDRS {
    pmusram_text PT_LOAD FLAGS(5 /* R_E */);
    pmusram_data PT_LOAD FLAGS(6 /* RW_ */);
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
        ASSERT(. == ALIGN(64 * 1024), ".pmusram.entry request 64K aligned.")

        KEEP(*(.pmusram.entry))

        __bl31_pmusram_text_start = .;
        *(.pmusram.text)
        *(.pmusram.rodata)
        __bl31_pmusram_text_end = .;
    } >pmusram AT> pmusram :pmusram_text /* FIXME: what is the real LMA? */

    .pmusram.data : {
        __bl31_pmusram_data_start = .;
        *(.pmusram.data)
        __bl31_pmusram_data_end = .;
    } >pmusram AT>pmusram :pmusram_data /* FIXME: what is the real LMA? */
}

#endif /* __ROCKCHIP_PLAT_LD_S__ */
