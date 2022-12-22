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

/*
 * Our C runtime places constraints on how some output sections may be aligned,
 * described here.
 */
#define CRT_DATA_ALIGN 16 /* The `.data` section must be 16-byte aligned */
#define CRT_BSS_ALIGN 16 /* The `.bss` section must be 16-byte aligned */

MEMORY {
    ROM (rx): ORIGIN = BL1_RO_BASE, LENGTH = BL1_RO_LIMIT - BL1_RO_BASE
    RAM (w): ORIGIN = BL1_RW_BASE, LENGTH = BL1_RW_LIMIT - BL1_RW_BASE
}

SECTIONS {
    /*
     * The Procedure Linkage Table (PLT).
     *
     * When the image is loaded by a dynamic linker, this table is populated
     * with trampolines which look up a symbol in the PLT GOT, resolve its
     * absolute address if necessary, and then jump to it.
     *
     * BL1 doesn't support dynamic linkage, so we discard these sections.
     */
    /DISCARD/ /* .plt */ : { *(.plt) }
    /DISCARD/ /* .iplt */ : { *(.iplt) }

    /*
     * Relocation table for the PLT section.
     *
     * When the image is loaded by a dynamic linker, this table contains
     * relocation information for functions in the PLT.
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
     * BL1 doesn't support dynamic linkage, so we discard these sections.
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
     * BL1 doesn't support dynamic linkage, so we discard these sections.
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
     * BL1 doesn't support exception handling, so we discard these sections.
     */
    /DISCARD/ /* .ARM.extab */ : { *(.ARM.extab* .gnu.linkonce.armextab.*) }

    /*
     * Arm ELF32 exception-handling index table.
     *
     * When exception handling is enabled, this table contains the offsets of
     * every entry in the exception-handling table given in the same order as
     * the addresses of their associated functions.
     *
     * BL1 doesn't support exception handling, so we discard these sections.
     */
    /DISCARD/ /* .ARM.exidx */ : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
}

SECTIONS {
    /*
     * Miscellaneous comments with no specific purpose, represented by a table
     * of null-terminated strings. Often used to attach information about the
     * toolchain to the binary.
     */
    .comment 0 : { *(.comment) }

    /* DWARF 1.1 debug information */
    .debug_aranges 0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }

    /* DWARF 2 debug information */
    .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_line 0 : { *(.debug_line .debug_line.* .debug_line_end) }
    .debug_frame 0 : { *(.debug_frame) }
    .debug_str 0 : { *(.debug_str) }
    .debug_loc 0 : { *(.debug_loc) }
    .debug_macinfo 0 : { *(.debug_macinfo) }

    /* DWARF 3 debug information  */
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
    .debug_ranges 0 : { *(.debug_ranges) }

    /*
     * Arm/GNU build attributes.
     *
     * Build attributes record data that a linker needs to reason mechanically
     * about the compatibility, or incompatibility, of a set of relocatable
     * files.
     */
    .ARM.attributes 0 : {
        KEEP(*(.ARM.attributes))
        KEEP(*(.gnu.attributes))
    }
}

SECTIONS {
    . = ORIGIN(ROM);

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL1 ROM address not aligned on a page boundary")

    /* Read-only executable data */
    .text : {
        /*
         * Exception vector table.
         *
         * This is a table of AArch32/AArch64 exception vector entries, which
         * are small trampolines that jump into the relevant exception handler.
         *
         * The exception handlers themselves live in standard `.text` sections.
         */
        *(.vectors)

        /*
         * Read-only executable data.
         *
         * Note that functions on the hot path are allocated together, which can
         * allow us to take advantage of cache locality after profiling.
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
         * This is an interesting mechanic introduced by GNU LD which allows
         * arbitrary linker warnings to be triggered when a particular symbol is
         * linked.
         */
        *(.gnu.warning .gnu.warning.*)

        /*
         * Arm/Thumb interworking code.
         *
         * These sections contain glue code to support Arm/Thumb interworking
         * on mixed-mode architectures prior to ARMv5T.
         *
         * We only support ARMv7-A or later, but the linker generates stub
         * sections anyway.
         */
        *(.glue_7t)
        *(.glue_7)

        /*
         * VFP11 coprocessor erratum veneer.
         *
         * This section contains a veneer to work around an erratum in the VFP11
         * coprocessor, enabled via `--vfp11-denorm-fix` in GNU LD.
         *
         * We don't support any processor with which the VFP11 might be used,
         * but the linker generates a stub section anyway.
         */
        *(.vfp11_veneer)

        /*
         * BX instruction support for ARMv4.
         *
         * This section contains a veneer which enables Thumb interworking
         * between code compiled for ARMv4 and ARMv4T, while also allowing the
         * callee to remain ARMv4-compatible.
         *
         * We don't support any processor with which the VFP11 might be used,
         * but the linker generates a stub section anyway.
         */
        *(.v4_bx)
    } >ROM

    /*
     * If we've been asked to keep executable and non-executable data in
     * separate pages, pad the rest of the page to ensure that no non-executable
     * data is mapped into an executable page.
     */
#if SEPARATE_CODE_AND_RODATA
    . = ALIGN(PAGE_SIZE);
#endif /* SEPARATE_CODE_AND_RODATA */

    __TEXT_START__ = ADDR(.text);
    __TEXT_END__ = .;

    /* Read-only non-executable data */
    .rodata : {
        /* Read-only non-executable data */
        *(.rodata .rodata.* .gnu.linkonce.r.*)

#if PLAT_RO_XLAT_TABLES
        /* Base translation tables */
        __BASE_XLAT_TABLE_START__ = .;
        *(base_xlat_table)
        __BASE_XLAT_TABLE_END__ = .;
#endif /* PLAT_RO_XLAT_TABLES */

        /* CPU operations */
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
     * runtime. There's no syntax to specify the alignment of the LMA of an
     * output section, but aligning the location counter has the same effect.
     */
    . = ALIGN(CRT_DATA_ALIGN);

    __BL1_ROM_START__ = ORIGIN(ROM);
    __BL1_ROM_END__ = __DATA_ROM_START__ + __DATA_SIZE__;
}

SECTIONS {
    . = ORIGIN(RAM);

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL1 RAM address not aligned on a page boundary")

    /* Read/write non-executable data */
    .data : ALIGN(CRT_DATA_ALIGN) {
        /* Read/write non-executable data */
        *(.data .data.* .gnu.linkonce.d.*)
    } >RAM AT>ROM

    __DATA_ROM_START__ = LOADADDR(.data);
    __DATA_RAM_START__ = ADDR(.data);
    __DATA_RAM_END__ = .;
    __DATA_SIZE__ = __DATA_RAM_END__ - __DATA_RAM_START__;

    /* Zero-initialized non-executable data */
    .bss (NOLOAD) : ALIGN(CRT_BSS_ALIGN) {
        /* Zero-initialized non-executable data */
        *(.dynbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)

#if !PLAT_RO_XLAT_TABLES
        /* Base translation tables */
        __BASE_XLAT_TABLE_START__ = .;
        *(base_xlat_table)
        __BASE_XLAT_TABLE_END__ = .;
#endif /* !PLAT_RO_XLAT_TABLES */

        /* Translation tables */
        __XLAT_TABLE_START__ = .;
        *(xlat_table)
        __XLAT_TABLE_END__ = .;
    } >RAM

    __BSS_START__ = ADDR(.bss);
    __BSS_END__ = .;

    /* Per-core stack space */
    .stacks (NOLOAD) : {
        /* Per-core stack space */
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >RAM

#if USE_COHERENT_MEM
    /*
     * Coherent memory region.
     *
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored in their own pages and are
     * not mixed with normal data. This is required to set up the correct memory
     * attributes for the coherent data page tables.
     */
    .coherent_ram (NOLOAD) : ALIGN(PAGE_SIZE) {
        *(tzfw_coherent_mem)
    } >RAM

    /*
     * Memory page(s) mapped to this section will be marked as device memory; no
     * other unexpected data must creep in. Ensure the rest of the current
     * memory page is unused.
     */
    . = ALIGN(PAGE_SIZE);

    __COHERENT_RAM_START__ = ADDR(.coherent_ram);
    __COHERENT_RAM_END_UNALIGNED__ = __COHERENT_RAM_START__ + SIZEOF(.coherent_ram);
    __COHERENT_RAM_END__ = .;
#endif /* USE_COHERENT_MEM */

    __BL1_RAM_START__ = ORIGIN(RAM);
    __BL1_RAM_END__ = .;
}
