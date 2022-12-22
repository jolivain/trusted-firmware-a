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

    /* DWARF 1.1 debug information */
    .debug_aranges : { *(.debug_aranges) }
    .debug_pubnames : { *(.debug_pubnames) }

    /* DWARF 2 debug information */
    .debug_info : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev : { *(.debug_abbrev) }
    .debug_line : { *(.debug_line .debug_line.* .debug_line_end) }
    .debug_frame : { *(.debug_frame) }
    .debug_str : { *(.debug_str) }
    .debug_loc : { *(.debug_loc) }
    .debug_macinfo : { *(.debug_macinfo) }

    /* DWARF 3 debug information  */
    .debug_pubtypes : { *(.debug_pubtypes) }
    .debug_ranges : { *(.debug_ranges) }

    /*
     * Arm ELF build attributes.
     *
     * Build attributes record data that a linker needs to reason mechanically
     * about the compatibility, or incompatibility, of a set of relocatable
     * files.
     */
    .ARM.attributes : {
        KEEP(*(.ARM.attributes))
        KEEP(*(.gnu.attributes))
    }

    /*
     * The Procedure Linkage Table (PLT).
     *
     * When the image is loaded by a dynamic linker, this table is populated
     * with trampolines which look up a symbol in the PLT GOT, resolve its
     * absolute address if necessary, then jump to it.
     *
     * BL1 doesn't support dynamic linkage, so we discard this section.
     */
    /DISCARD/ /* .plt */ : { *(.plt) }
    /DISCARD/ /* .iplt */ : { *(.iplt) }

    /*
     * Relocation table for the PLT section.
     *
     * When the image is loaded by a dynamic linker, this table contains
     * relocation information for functions in the PLT,
     *
     * BL1 doesn't support dynamic linkage, so we discard these sections.
     */
    /DISCARD/ /* .rel.iplt */ : { *(.rel.iplt) }
    /DISCARD/ /* .rela.iplt */ : { *(.rela.iplt) }
    /DISCARD/ /* .rel.plt */ : { *(.rel.plt) }
    /DISCARD/ /* .rela.plt */ : { *(.rela.plt) }

    /*
     * Relocation tables for other sections.
     *
     * When the image is loaded by a dynamic linker, these tables contain
     * relocation information for symbols not in the PLT.
     *
     * BL1 doesn't support dynamic linkage, so relocation information is
     * discarded.
     */
    /DISCARD/ /* .rel.init */ : { *(.rel.init) }
    /DISCARD/ /* .rela.init */ : { *(.rela.init) }
    /DISCARD/ /* .rel.text */ : { *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*) }
    /DISCARD/ /* .rela.text */ : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) }
    /DISCARD/ /* .rel.fini */ : { *(.rel.fini) }
    /DISCARD/ /* .rela.fini */ : { *(.rela.fini) }
    /DISCARD/ /* .rel.rodata */ : { *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) }
    /DISCARD/ /* .rela.rodata */ : { *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*) }
    /DISCARD/ /* .rel.data.rel.ro */ : { *(.rel.data.rel.ro .rel.data.rel.ro.* .rel.gnu.linkonce.d.rel.ro.*) }
    /DISCARD/ /* .rela.data.rel.ro */ : { *(.rela.data.rel.ro .rela.data.rel.ro.* .rela.gnu.linkonce.d.rel.ro.*) }
    /DISCARD/ /* .rel.data */ : { *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*) }
    /DISCARD/ /* .rela.data */ : { *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*) }
    /DISCARD/ /* .rel.tdata */ : { *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*) }
    /DISCARD/ /* .rela.tdata */ : { *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*) }
    /DISCARD/ /* .rel.tbss */ : { *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*) }
    /DISCARD/ /* .rela.tbss */ : { *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*) }
    /DISCARD/ /* .rel.ctors */ : { *(.rel.ctors) }
    /DISCARD/ /* .rela.ctors */ : { *(.rela.ctors) }
    /DISCARD/ /* .rel.dtors */ : { *(.rel.dtors) }
    /DISCARD/ /* .rela.dtors */ : { *(.rela.dtors) }
    /DISCARD/ /* .rel.got */ : { *(.rel.got) }
    /DISCARD/ /* .rela.got */ : { *(.rela.got) }
    /DISCARD/ /* .rel.bss */ : { *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*) }
    /DISCARD/ /* .rela.bss */ : { *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*) }

    /*
     * Global Offset Tables (GOT).
     *
     * When the image is loaded by a dynamic linker, these tables are filled in
     * at runtime with the real addresses of symbols resolved by the linker.
     *
     * BL1 doesn't support dynamic linkage, so relocation information is
     * discarded.
     */
    /DISCARD/ /* .got */ : { *(.got) *(.igot) }
    /DISCARD/ /* .got.plt */ : { *(.got.plt) *(.igot.plt) }

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

        /*
         * Arm/Thumb interworking code.
         *
         * These sections contain glue code to support Arm/Thumb interworking
         * on mixed-mode architectures prior to ARMv5T.
         *
         * The linker generates stub sections regardless of the fact that we
         * don't actually need them, so place them here.
         */
        *(.glue_7t)
        *(.glue_7)

        /*
         * VFP11 coprocessor erratum veneer.
         *
         * This section contains a veneer to work around an erratum in the VFP11
         * coprocessor, enabled via `--vfp11-denorm-fix` in GNU LD.
         *
         * The linker generates a stub section regardless of the fact that we
         * don't actually need it, so place it here.
         */
        *(.vfp11_veneer)

        /*
         * BX instruction support for ARMv4.
         *
         * This section contains a veneer which enables Thumb interworking
         * between code compiled for ARMv4 and ARMv4T, while also allowing the
         * callee to remain ARMv4-compatible.
         *
         * The linker generates a stub section regardless of the fact that we
         * don't actually need it, so place it here.
         */
        *(.v4_bx)
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
