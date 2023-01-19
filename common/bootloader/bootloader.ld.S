/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)

/*
 * Note that output sections in this file are ordered in roughly descending
 * order of an alignment in order to save as much space from padding as
 * possible.
 *
 * There are some exceptions, such as in regions where two different groups are
 * unlikely to be non-contiguous and where it doesn't matter if they share a
 * page.
 */

/*
 * The linker has an innate understanding of pages, and defines a couple of
 * constants for convenience. Make sure the preprocessor agrees with them.
 */
ASSERT(CONSTANT(COMMONPAGESIZE) == PAGE_SIZE, "page size mismatch");
ASSERT(CONSTANT(MAXPAGESIZE) == PAGE_SIZE, "page size mismatch");

/*
 * Read-only executable sections.
 *
 * These sections contribute to the executable segment of the bootloader image,
 * also known as the text or code segment.
 */
SECTIONS {
    . = ORIGIN(x);

    __X_START__ = .;
    __TEXT_START__ = .;

    /*
     * Bootloader entrypoints.
     *
     * In every bootloader image the entrypoint is placed at the beginning of
     * the image, so that it is predictable for the previous bootloader.
     */
    .text.entrypoint : {
        *(.text.asm.bl*_entrypoint)
    }

    /* Read-only executable data */
    .text : {
        *(.text .text.* .gnu.linkonce.t.*)

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
    }

    /*
     * The Procedure Linkage Table (PLT).
     *
     * The PLT is a table of trampolines to position-independent functions which
     * are bound lazily (i.e. on demand, when the function is first called).
     *
     * Indirect function calls via the PLT actually invoke the associated
     * trampoline, which coordinates with the dynamic linker to resolve the GOT
     * entry for the function before jumping to it.
     *
     * We do not support lazy binding, so we do not expect a PLT.
     */
    .text.plt : {
        *(.plt) *(.iplt)
    }

    ASSERT(SIZEOF(.text.plt) == 0, "unexpected PLT found")

    /*
     * Exception vector table.
     *
     * This is a table of AArch32/AArch64 exception vector entries, which
     * are small trampolines that jump into the relevant exception handler.
     *
     * The exception handlers themselves live in standard `.text` sections.
     */
    .text.vectors : {
        *(.vectors)
    }

    . = .;

    __TEXT_END__ = .;
    __X_END__ = .;
}

/*
 * Read-only data sections.
 *
 * These sections contribute to the read-only data segment of the bootloader
 * image.
 */
SECTIONS {
#if SEPARATE_CODE_AND_RODATA
    /*
        * When the executable segment and the read-only data segment would
        * otherwise be contiguous, we can separate the two to prevent any
        * non-executable data pages from being marked as executable.
        *
        * In this case, we align the read-only data sections to the next page
        * boundary by padding the final executable section to the end of the
        * page.
        */

    . = (ORIGIN(r) == ORIGIN(x) ? ALIGN(CONSTANT(MAXPAGESIZE)) : .);
#else
    . = (ORIGIN(r) == ORIGIN(x) ? . : ORIGIN(r));
#endif /* SEPARATE_CODE_AND_RODATA */

    __R_START__ = .;
    __RODATA_START__ = .;

}

#if PLAT_RO_XLAT_TABLES
#   include "fragments/base_xlat_table.ld.S"
#endif /* PLAT_RO_XLAT_TABLES */

SECTIONS {
    /* CPU operations table */
    .rodata.cpu_ops : {
        __CPU_OPS_START__ = .;
        KEEP(*(cpu_ops))
        __CPU_OPS_END__ = .;
    }

    /* SPMC Logical Partition descriptor table */
    .rodata.el3_lp_descs : {
        __EL3_LP_DESCS_START__ = .;
        KEEP(*(el3_lp_descs))
        __EL3_LP_DESCS_END__ = .;
    }

    /* Firmware Configuration Framework (FCONF) populator table */
    .rodata.fconf_populator : {
        __FCONF_POPULATOR_START__ = .;
        KEEP(*(.fconf_populator))
        __FCONF_POPULATOR_END__ = .;
    }

    /* Image Parsing library descriptor table */
    .rodata.img_parser_lib_descs : {
        __PARSER_LIB_DESCS_START__ = .;
        KEEP(*(.img_parser_lib_descs))
        __PARSER_LIB_DESCS_END__ = .;
    }

    /* Performance Measurement Framework (PMF) service descriptor table */
    .rodata.pmf_svc_descs : {
        __PMF_SVC_DESCS_START__ = .;
        KEEP(*(pmf_svc_descs))
        __PMF_SVC_DESCS_END__ = .;
    }

    /* Runtime service descriptor table */
    .rodata.rt_svc_descs : {
        __RT_SVC_DESCS_START__ = .;
        KEEP(*(rt_svc_descs))
        __RT_SVC_DESCS_END__ = .;
    }

    /*
     * Global Offset Table (GOT).
     *
     * The GOT is a table mapping position-independent symbols to their absolute
     * addresses in memory. This table would updated by the a dynamic linker if
     * one were available, but instead we do it ourselves.
     */
    .rodata.got : {
        __GOT_START__ = .;
        *(.got)
        __GOT_END__ = .;
    }

#ifndef __PIE__
    ASSERT(SIZEOF(.rodata.got) == 0, "unexpected GOT found")
#endif

    /*
     * Global Offset Table (GOT) for PLT entries.
     *
     * This table follows the same scheme as the section above, except it
     * contains entries which are patched by the trampolines in the Procedure
     * Linkage Table (PLT).
     *
     * Because we don't support lazy binding there is no PLT to patch this GOT,
     * and we therefore expect it to be empty.
     */
    .rodata.got.plt : {
        *(.igot.plt)
    }

    ASSERT(SIZEOF(.rodata.got.plt) == 0, "unexpected GOT PLT entries found")

    /*
     * REL-type relocations.
     *
     * This table contains REL-type relocation information for symbols which
     * need to be relocated (usually by a dynamic linker, but by ourself in this
     * case).
     */
    .rodata.rel.dyn : {
        __REL_DYN_START__ = .;
        *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
        __REL_DYN_END__ = .;
    }

#ifndef __PIE__
    ASSERT(SIZEOF(.rodata.rel.dyn) == 0, "unexpected relocations found")
#endif

    /*
     * REL-type relocations for PLT entries.
     *
     * This table contains REL-type relocation information for Global Offset
     * Tables (GOT) entries which are patched by the trampolines in the
     * Procedure Linkage Table (PLT).
     *
     * Because we don't support lazy binding there is no PLT to patch this GOT,
     * and we therefore expect it to be empty.
     */
    .rodata.rel.plt : {
        *(.rel.plt)
    }

    ASSERT(SIZEOF(.rodata.rel.plt) == 0, "unexpected PLT relocations found")

    /*
     * RELA-type relocations.
     *
     * This table contains RELA-type relocation information for symbols which
     * need to be relocated (usually by a dynamic linker, but by ourself in this
     * case).
     */
    .rodata.rela.dyn : {
        __RELA_DYN_START__ = .;
        *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
        __RELA_DYN_END__ = .;
    }

#ifndef __PIE__
    ASSERT(SIZEOF(.rodata.rela.dyn) == 0, "unexpected relocations found")
#endif

    /*
     * RELA-type relocations for PLT entries.
     *
     * This table contains RELA-type relocation information for symbols with PLT
     * entries which need to be relocated (usually by a dynamic linker, but by
     * ourself in this case).
     */
    .rodata.rela.plt : {
        *(.rela.iplt)
    }

    ASSERT(SIZEOF(.rodata.rela.plt) == 0, "unexpected PLT relocations found")

    /*
     * We only support REL-type relocations in AArch32 builds, and RELA-type
     * relocations in AArch64 builds.
     */
#ifdef __aarch64__
    ASSERT(SIZEOF(.rodata.rel.dyn) == 0,
        "unexpected REL-type relocations found")

    __RELA_START__ = __RELA_DYN_START__;
    __RELA_END__ = __RELA_DYN_END__;
#else /* __aarch64__ */
    ASSERT(SIZEOF(.rodata.rela.dyn) == 0,
        "unexpected RELA-type relocations found")

    __RELA_START__ = __REL_DYN_START__;
    __RELA_END__ = __REL_DYN_END__;
#endif /* __aarch64__ */

    /*
     * Arm ELF32 exception-handling table.
     *
     * This table contains variable-size entries encoding, in a vendor- and
     * language-specific way, the actions required to propagate an exception
     * through a function.
     */
    .rodata.ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    }

    /*
     * Arm ELF32 exception-handling index table.
     *
     * When exception handling is enabled, this table contains the offsets of
     * every entry in the exception-handling table given in the same order as
     * the addresses of their associated functions.
     */
    .rodata.ARM.exidx : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    }

    /* Read-only data */
    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    }

    ASSERT(SIZEOF(.rodata.ARM.extab) == 0,
        "unexpected exception handler table found")
    ASSERT(SIZEOF(.rodata.ARM.exidx) == 0,
        "unexpected exception handler index table found")

    . = .;

    __RODATA_END__ = .;
    __R_END__ = __DATA_LOAD_END__;
}

/*
 * Writable data sections.
 *
 * These sections contribute to the writable data segment of the bootloader
 * image.
 */
SECTIONS {
    . = (ORIGIN(w) == ORIGIN(r) ? . : ORIGIN(w));
    . = ALIGN(DATA_ALIGN);

    __W_START__ = .;
    __DATA_START__ = .;

    /* Writable data */
    .data : AT(ALIGN(__RODATA_END__, DATA_ALIGN) + (ADDR(.data) - __DATA_START__)) {
        *(.data .data.* .gnu.linkonce.d.*)
    }

    . = ALIGN(DATA_ALIGN);

    __DATA_SIZE__ = __DATA_END__ - __DATA_START__;

    __DATA_LOAD_START__ = LOADADDR(.data) - (ADDR(.data) - __DATA_START__);
    __DATA_LOAD_END__ = __DATA_LOAD_START__ + __DATA_SIZE__;

    __DATA_END__ = .;
    __W_END__ = .;
}

/*
 * Zero-initialized data sections.
 *
 * These sections contribute to the writable data segment of the bootloader
 * image, and zero-initialized by the C runtime.
 */
SECTIONS {
    . = (ORIGIN(n) == ORIGIN(w) ? . : ORIGIN(n));
    . = ALIGN(BSS_ALIGN);

    __N_START__ = .;
    __BSS_START__ = .;
}

#if !PLAT_RO_XLAT_TABLES
#   include "fragments/base_xlat_table.ld.S"
#endif /* PLAT_RO_XLAT_TABLES */

SECTIONS {
    /*
     * Performance Measurement Framework timestamps.
     *
     * The compiler will allocate enough memory for one core's timestamps, but
     * the remaining memory for other cores is allocated manually here.
     */
    .bss.pmf_timestamps (NOLOAD) : ALIGN(CACHE_WRITEBACK_GRANULE) {
        __PMF_TIMESTAMP_START__ = .;

        KEEP(*(pmf_timestamp_array))

        . = ALIGN(CACHE_WRITEBACK_GRANULE);

        __PMF_PERCPU_TIMESTAMP_END__ = .;
        __PERCPU_TIMESTAMP_SIZE__ = ABSOLUTE(. - __PMF_TIMESTAMP_START__);

        . += (__PERCPU_TIMESTAMP_SIZE__ * (PLATFORM_CORE_COUNT - 1));

        __PMF_TIMESTAMP_END__ = .;
    }

    /* Bakery locks */
#if !USE_COHERENT_MEM
    .bss.bakery_locks (NOLOAD) : ALIGN(CACHE_WRITEBACK_GRANULE) {
        __BAKERY_LOCK_START__ = .;
        __PERCPU_BAKERY_LOCK_START__ = .;

        *(bakery_lock)

        . = ALIGN(CACHE_WRITEBACK_GRANULE);

        __PERCPU_BAKERY_LOCK_END__ = .;
        __PERCPU_BAKERY_LOCK_SIZE__ =
            __PERCPU_BAKERY_LOCK_END__ - __PERCPU_BAKERY_LOCK_START__;

        . += (__PERCPU_BAKERY_LOCK_SIZE__ * (PLATFORM_CORE_COUNT - 1));

        __BAKERY_LOCK_END__ = .;
    }

#   ifdef PLAT_PERCPU_BAKERY_LOCK_SIZE
    ASSERT(
        (__PERCPU_BAKERY_LOCK_SIZE__ == 0) ||
            (__PERCPU_BAKERY_LOCK_SIZE__ == PLAT_PERCPU_BAKERY_LOCK_SIZE),
        "per-core bakery lock size does not meet requirements")
#   endif /* PLAT_PERCPU_BAKERY_LOCK_SIZE */
#endif /* !USE_COHERENT_MEM */

    /* Zero-initialized data */
    .bss (NOLOAD) : {
        /* Zero-initialized data */
        *(.bss .bss.* .gnu.linkonce.b.*)
    }

    . = ALIGN(BSS_ALIGN);

    __BSS_END__ = .;
}

/*
 * Uninitialized data sections.
 *
 * These sections contribute to the writable data segment of the bootloader
 * image, and are used as-is without initialization.
 *
 * Keep in mind that it is undefined behaviour for a C program to read from
 * objects placed in this memory without explicitly initializing it.
 */
SECTIONS {
    __NOINIT_START__ = .;

#if USE_COHERENT_MEM
    /*
     * Coherent memory region.
     *
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored in their own pages and are
     * not mixed with normal data. This is required to set up the correct memory
     * attributes for the coherent data page tables.
     */
    .noinit.coherent_ram (NOLOAD) : ALIGN(CONSTANT(MAXPAGESIZE)) {
        __COHERENT_RAM_START__ = .;
        *(tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to this section will be marked as device
         * memory; no other unexpected data must creep in. Ensure the rest of
         * the current memory page is unused.
         */
        . = ALIGN(CONSTANT(MAXPAGESIZE));

        __COHERENT_RAM_END__ = .;
    }
#endif /* USE_COHERENT_MEM */

    /*
     * Translation tables.
     *
     * The translation tables configure the translation regime for the MMU.
     * We place them first here because they have a large alignment requirement,
     * which saves us some space which otherwise would have been padded.
     */
    .noinit.xlat_tables (NOLOAD) : {
        __XLAT_TABLE_START__ = .;
        *(xlat_table)
        __XLAT_TABLE_END__ = .;
    }

    /*
     * Per-core stack space.
     *
     * Similar to the translation tables, the stack space has a fairly strict
     * alignment requirement. We place this section immediately after the
     * translation tables to again save ourselves some padding.
     */
    .noinit.stacks (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    }

    /* Uninitialized data */
    .noinit (NOLOAD) : {
        *(.noinit .noinit.* .gnu.linkonce.n.*)
    }

    . = .;

    __NOINIT_END__ = .;
    __N_END__ = .;
}

/*
 * Metadata sections.
 *
 * These sections are not allocated, and consist purely of metadata. Most of
 * this metadata is intended for debugging and other tooling.
 */
SECTIONS {
    .symtab 0 : { *(.symtab) }
    .strtab 0 : { *(.strtab) }
    .shstrtab 0 : { *(.shstrtab) }

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
     * Miscellaneous comments with no specific purpose, represented by a table
     * of null-terminated strings. Often used to attach information about the
     * toolchain to the binary.
     */
    .comment 0 : { *(.comment) }

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
