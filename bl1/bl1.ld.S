/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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
    /DISCARD/ /* .plt */ : { *(.plt .iplt) }

    /*
     * Relocation table for the PLT section.
     *
     * When the image is loaded by a dynamic linker, this table contains
     * relocation information for functions in the PLT,
     *
     * BL1 doesn't support dynamic linkage, so we discard this section.
     */
    /DISCARD/ /* .rela.plt */ : { *(.rela.plt .rela.iplt) }

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
    /DISCARD/ /* .got.plt */ : { *(.got.plt .igot.plt) }

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
    /DISCARD/ /* .ARM.extab */ : { *(.ARM.extab* .gnu.linkonce.armextab.*) }

    /*
     * Arm ELF32 exception-handling index table.
     *
     * When exception handling is enabled, this table contains the offsets of
     * every entry in the exception-handling table given in the same order as
     * the addresses of their associated functions.
     *
     * BL1 doesn't support exception handling, so exception-handling information
     * is discarded.
     */
    /DISCARD/ /* .ARM.exidx */ : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
}

SECTIONS {
    . = ORIGIN(ROM);

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL1 ROM address not aligned on a page boundary")

    .text : {
        /*
         * Exception vector table.
         *
         * This is a table of AArch32/AArch64 exception vector entries, which
         * are small trampolines that jump into the relevant exception handler.
         */
        *(.vectors)

        /*
         * Executable data.
         *
         * Note that functions on the cold path are placed first, further away
         * from hotter functions, which can take advantage of cache locality and
         * more compact relative addressing instructions.
         */
        *(.text.unlikely .text.*_unlikely .text.unlikely.*)
        *(.text.exit .text.exit.*)
        *(.text.startup .text.startup.*)
        *(.text.hot .text.hot.*)
        *(SORT_BY_NAME(.text.sorted.*))
        *(.text .stub .text.* .gnu.linkonce.t.*)

        /*
         * GNU LD linker warnings.
         *
         * This is an interesting mechanic introduced by GNU LD which allows the
         * developer to trigger arbitrary linker warnings when a particular
         * symbol is linked.
         */
        *(.gnu.warning .gnu.warning.*)
    } >ROM

    /*
     * If we've been asked to keep executable and non-executable data separate,
     * zero the rest of the page. This prevents non-executable data from
     * overlapping an executable page.
     */
#if SEPARATE_CODE_AND_RODATA
    . = ALIGN(PAGE_SIZE);
#endif /* SEPARATE_CODE_AND_RODATA */

    __TEXT_START__ = ADDR(.text);
    __TEXT_END__ = .;

    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r.*)

#if PLAT_RO_XLAT_TABLES
        __BASE_XLAT_TABLE_START__ = .;
        *(base_xlat_table)
        __BASE_XLAT_TABLE_END__ = .;
#endif /* PLAT_RO_XLAT_TABLES */

        __CPU_OPS_START__ = .;
        KEEP(*(cpu_ops))
        __CPU_OPS_END__ = .;
    } >ROM

    __RODATA_START__ = ADDR(.rodata);
    __RODATA_END__ = .;

    __RO_START__ = __TEXT_START__;
    __RO_END__ = __RODATA_END__;

    /*
     * Read/write data is copied from ROM to RAM during initialization of the C
     * runtime. Our CRT implementation requires that the LMA be 16-byte aligned
     * to allow us to copy in 16-byte blocks without alignment checks.
     */
    . = ALIGN(16);

    __BL1_ROM_START__ = ORIGIN(ROM);
    __BL1_ROM_END__ = __DATA_ROM_START__ + __DATA_SIZE__;
}

SECTIONS {
    . = ORIGIN(RAM);

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL1 RAM address not aligned on a page boundary")

    .data : ALIGN_WITH_INPUT {
        *(.data .data.* .gnu.linkonce.d.*)
    } >RAM AT>ROM

    __DATA_SIZE__ = __DATA_RAM_END__ - __DATA_RAM_START__;
    __DATA_ROM_START__ = LOADADDR(.data);
    __DATA_RAM_START__ = ADDR(.data);
    __DATA_RAM_END__ = .;

    /*
     * There's no need to pad out the read-write segment to a page boundary
     * because zero-initialized data can be safely allocated within the same
     * page (it's all non-executable read-write).
     */

    .bss (NOLOAD) : {
        *(.dynbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)

#if !PLAT_RO_XLAT_TABLES
        __BASE_XLAT_TABLE_START__ = .;
        *(base_xlat_table)
        __BASE_XLAT_TABLE_END__ = .;
#endif /* !PLAT_RO_XLAT_TABLES */

        __XLAT_TABLE_START__ = .;
        *(xlat_table)
        __XLAT_TABLE_END__ = .;
    } >RAM

    __BSS_START__ = ADDR(.bss);
    __BSS_END__ = .;

    /*
     * The stacks for each core are allocated by the `tzfw_normal_stacks` input
     * sections. The contents of these sections are already aligned, so we don't
     * need to specify an explicit alignment.
     */
    .stacks (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >RAM

#if USE_COHERENT_MEM
    /*
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored in their own pages and are
     * not mixed with normal data. This is required to set up the correct memory
     * attributes for the coherent data page tables.
     */
    .coherent_ram (NOLOAD) : ALIGN(PAGE_SIZE) {
        *(tzfw_coherent_mem)
    } >RAM

    /*
     * Memory page(s) mapped to this section will be marked as device
     * memory. No other unexpected data must creep in. Ensure the rest of
     * the current memory page is unused.
     */
    . = ALIGN(PAGE_SIZE);

    __COHERENT_RAM_START__ = ADDR(.coherent_ram);
    __COHERENT_RAM_END_UNALIGNED__ = __COHERENT_RAM_START__ + SIZEOF(.coherent_ram);
    __COHERENT_RAM_END__ = .;
#endif /* USE_COHERENT_MEM */

    __BL1_RAM_START__ = ORIGIN(RAM);
    __BL1_RAM_END__ = .;
}
