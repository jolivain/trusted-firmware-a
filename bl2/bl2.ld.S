/*
 * Copyright (c) 2013-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(bl2_entrypoint)

MEMORY {
    RAM (rwx): ORIGIN = BL2_BASE, LENGTH = BL2_LIMIT - BL2_BASE
}

SECTIONS {
    . = BL2_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL2_BASE address is not aligned on a page boundary.")

    __RO_START__ = .;
    __TEXT_START__ = .;

    .text . : {
#if ENABLE_RME
        *bl2_rme_entrypoint.o(.text*)
#else /* ENABLE_RME */
        *bl2_entrypoint.o(.text*)
#endif /* ENABLE_RME */

        *(SORT_BY_ALIGNMENT(.text*))
        *(.vectors)
    } >RAM

#if SEPARATE_CODE_AND_RODATA
    . = ALIGN(PAGE_SIZE);
#endif /* SEPARATE_CODE_AND_RODATA */

    __TEXT_END__ = .;
    __RODATA_START__ = .;

    .rodata . : {
        *(SORT_BY_ALIGNMENT(.rodata*))

        RODATA_COMMON
    } >RAM

    .ARM.extab . : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >RAM

    .ARM.exidx . : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >RAM

    . = ALIGN(PAGE_SIZE);

    __RODATA_END__ = .;
    __RO_END__ = .;

    __RW_START__ = .;

    DATA_SECTION >RAM
    STACK_SECTION >RAM
    BSS_SECTION >RAM
    XLAT_TABLE_SECTION >RAM

#if USE_COHERENT_MEM
    /*
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored on their own pages and are
     * not mixed with normal data.  This is required to set up the correct
     * memory attributes for the coherent data page tables.
     */
    coherent_ram (NOLOAD) : ALIGN(PAGE_SIZE) {
        __COHERENT_RAM_START__ = .;
        *(tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to this section will be marked as device
         * memory. No other unexpected data must creep in. Ensure the rest of
         * the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);

        __COHERENT_RAM_END__ = .;
    } >RAM
#endif /* USE_COHERENT_MEM */

    __RW_END__ = .;
    __BL2_END__ = .;

    __BSS_SIZE__ = SIZEOF(.bss);

#if USE_COHERENT_MEM
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
#endif /* USE_COHERENT_MEM */

    ASSERT(. <= BL2_LIMIT, "BL2 image has exceeded its limit.")
}
