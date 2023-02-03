/*
 * Copyright (c) 2022-2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_RODATA_LD_S
#define PLAT_RODATA_LD_S

#include <lib/mtk_init/mtk_init.h>

SECTIONS {
    .rodata.mtk : {
        . = ALIGN(32);

        INIT_CALL_TABLE(EXPAND_AS_LINK_SECTION);
        __MTK_PLAT_INITCALL_END__ = .;

        . = ALIGN(32);

        __MTK_MMAP_POINTER_POOL_START__ = .;
        KEEP(*(.mtk_mmap_pool))
        __MTK_MMAP_POINTER_POOL_END_UNALIGNED__ = .;

        . = ALIGN(8);

        __MTK_MMAP_POOL_START__ = .;
        KEEP(*(.mtk_mmap_lists))
        __MTK_MMAP_POOL_END_UNALIGNED__ = .;

        . = ALIGN(32);

        __MTK_SMC_POOL_START__ = .;
        KEEP(*(.mtk_smc_descriptor_pool))
        __MTK_SMC_POOL_END_UNALIGNED__ = .;

        . = ALIGN(8);

#include <vendor_pubsub_events.h>

        *(.mtk_plat_ro)
    } >r AT>r :r
}

INSERT AFTER .rodata;

#endif /* PLAT_RODATA_LD_S */
