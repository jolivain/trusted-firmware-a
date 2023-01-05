/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
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
    /*
     * Miscellaneous comments with no specific purpose, represented by a table
     * of null-terminated strings. Often used to attach information about the
     * toolchain to the binary.
     */
    .comment : { *(.comment) }

    /*
     * DWARF 2 debug information. These sections must be placed, otherwise the
     * ELF image cannot be used for debugging.
     */
    .debug_info : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev : { *(.debug_abbrev) }
    .debug_line : { *(.debug_line .debug_line.* .debug_line_end) }
    .debug_frame : { *(.debug_frame) }
    .debug_str : { *(.debug_str) }
    .debug_loc : { *(.debug_loc) }
    .debug_macinfo : { *(.debug_macinfo) }

    /*
     * The Procedure Linkage Table (PLT).
     *
     * When the image is loaded by a dynamic linker, this table is populated
     * with trampolines which look up a symbol in the PLT GOT, resolve its
     * absolute address if necessary, then jump to it.
     *
     * BL1 doesn't support dynamic linkage, so we discard this section.
     */
    /DISCARD/ /* .plt */ : {
        *(.plt .iplt)
    }

    /*
     * Relocation table for the PLT section.
     *
     * When the image is loaded by a dynamic linker, this table contains
     * relocation information for functions in the PLT,
     *
     * BL1 doesn't support dynamic linkage, so we discard this section.
     */
    /DISCARD/ /* .rela.plt */ : {
        *(.rela.plt .rela.iplt)
    }

    /*
     * Relocation tables for other sections.
     *
     * When the image is loaded by a dynamic linker, these tables contain
     * relocation information for symbols not in the PLT.
     *
     * BL1 doesn't support dynamic linkage, so relocation information is
     * discarded.
     */
    /DISCARD/ /* .rela.dyn */ : {
        *(.rela.init)
        *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
        *(.rela.fini)
        *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*)
        *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)
        *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*)
        *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*)
        *(.rela.ctors)
        *(.rela.dtors)
        *(.rela.got)
        *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
        *(.rela.ifunc)
    }

    /*
     * Global Offset Table (GOT) of functions in the PLT.
     *
     * When the image is loaded by a dynamic linker, this table is filled in at
     * runtime with the real addresses of functions resolved by the linker.
     *
     * BL1 doesn't support dynamic linkage, so relocation information is
     * discarded.
     */
    /DISCARD/ /* .got.plt */ : {
        *(.got.plt .igot.plt)
    }

    /*
     * Arm ELF32 exception-handling table.
     *
     * When exception handling is enabled, this table contains variable-size
     * entries encoding, in a vendor- and language-specific way, the actions
     * required to propagate an exception through a function.
     *
     * BL1 doesn't support exception handling, so exception-handling information
     * is discarded.
     */
    /DISCARD/ /* .ARM.extab */ : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >ROM

    /*
     * Arm ELF32 exception-handling index table.
     *
     * When exception handling is enabled, this table contains the offsets of
     * every entry in the exception-handling table.
     *
     * BL1 doesn't support exception handling, so exception-handling information
     * is discarded.
     */
    /DISCARD/ /* .ARM.exidx */ : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >ROM
}

SECTIONS {
    . = BL1_RO_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL1_RO_BASE address is not aligned on a page boundary.")

    .text . : {
        __RO_START__ = .;
        __TEXT_START__ = .;

        *bl1_entrypoint.o(.text*)

        *(SORT_BY_ALIGNMENT(.text*))
        *(.vectors)

#if SEPARATE_CODE_AND_RODATA
        . = ALIGN(PAGE_SIZE);
#endif /* SEPARATE_CODE_AND_RODATA */

        __TEXT_END__ = .;
    } >ROM

    .rodata . : {
        __RODATA_START__ = .;

        *(SORT_BY_ALIGNMENT(.rodata*))

        RODATA_COMMON

        __RODATA_END__ = .;
        __RO_END__ = .;

        /*
         * No need to pad out the .rodata section to a page boundary. Next is
         * the .data section, which can mapped in ROM with the same memory
         * attributes as the .rodata section.
         *
         * Pad out to 16 bytes though as .data section needs to be 16-byte
         * aligned and lld does not align the LMA to the alignment specified
         * on the .data section.
         */
        . = ALIGN(16);
    } >ROM

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
