/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * The .data section gets copied from ROM to RAM at runtime. Its LMA should be
 * 16-byte aligned to allow efficient copying of 16-bytes aligned regions in it.
 * Its VMA must be page-aligned as it marks the first read/write page.
 */
#define DATA_ALIGN	16

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(bl1_entrypoint)

MEMORY {
    ROM (rx): ORIGIN = BL1_RO_BASE, LENGTH = BL1_RO_LIMIT - BL1_RO_BASE
    RAM (w): ORIGIN = BL1_RW_BASE, LENGTH = BL1_RW_LIMIT - BL1_RW_BASE
}

SECTIONS {
    . = BL1_RO_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL1_RO_BASE address is not aligned on a page boundary.")

    __RO_START__ = .;
    __TEXT_START__ = .;

    .text . : {
        *bl1_entrypoint.o(.text*)

        *(SORT_BY_ALIGNMENT(.text*))
        *(.vectors)
    } >ROM

#if SEPARATE_CODE_AND_RODATA
    . = ALIGN(PAGE_SIZE);
#endif /* SEPARATE_CODE_AND_RODATA */

    __TEXT_END__ = .;
    __RODATA_START__ = .;

    .rodata . : {
        *(SORT_BY_ALIGNMENT(.rodata*))

        RODATA_COMMON
    } >ROM

    .ARM.extab . : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >ROM

    .ARM.exidx . : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >ROM

    __RODATA_END__ = .;
    __RO_END__ = .;

    /*
     * No need to pad out the read-only segment to a page boundary, because the
     * read-write segment that follows it is not used until it has been copied
     * to read-write memory.
     *
     * Pad out to 16 bytes though, as the `.data` section needs to be 16-byte
     * aligned and LLVM does not align the LMA to the aligmnent specified on the
     * section.
     */
    . = ALIGN(16);

    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
        "cpu_ops not defined for this platform.")

    . = BL1_RW_BASE;

    ASSERT(BL1_RW_BASE == ALIGN(PAGE_SIZE),
        "BL1_RW_BASE address is not aligned on a page boundary.")

    DATA_SECTION >RAM AT>ROM

    __DATA_RAM_START__ = __DATA_START__;
    __DATA_RAM_END__ = __DATA_END__;

    STACK_SECTION >RAM
    BSS_SECTION >RAM
    XLAT_TABLE_SECTION >RAM

#if USE_COHERENT_MEM
    /*
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored on their own pages and are
     * not mixed with normal data. This is required to set up the correct memory
     * attributes for the coherent data page tables.
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

    __BL1_RAM_START__ = ADDR(.data);
    __BL1_RAM_END__ = .;

    __DATA_ROM_START__ = LOADADDR(.data);
    __DATA_SIZE__ = SIZEOF(.data);

    /*
     * The .data section is the last PROGBITS section so its end marks the end
     * of BL1's actual content in Trusted ROM.
     */
    __BL1_ROM_END__ =  __DATA_ROM_START__ + __DATA_SIZE__;

    ASSERT(__BL1_ROM_END__ <= BL1_RO_LIMIT,
        "BL1's ROM content has exceeded its limit.")

    __BSS_SIZE__ = SIZEOF(.bss);

#if USE_COHERENT_MEM
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
#endif /* USE_COHERENT_MEM */

    ASSERT(. <= BL1_RW_LIMIT, "BL1's RW section has exceeded its limit.")
}
