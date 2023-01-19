/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)

ASSERT(CONSTANT(MAXPAGESIZE) == PAGE_SIZE, "page size mismatch");

/*
 * Our C runtime places constraints on how some output sections may be aligned,
 * described here.
 */
#define CRT_DATA_ALIGN 16 /* The `.data` section must be 16-byte aligned */
#define CRT_BSS_ALIGN 16 /* The `.bss` section must be 16-byte aligned */

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
     * Exception vector table.
     *
     * This is a table of AArch32/AArch64 exception vector entries, which
     * are small trampolines that jump into the relevant exception handler.
     *
     * The exception handlers themselves live in standard `.text` sections.
     */
    .text.vectors . : {
        *(.vectors)
    } >x

    /* Read-only executable data */
    .text . : {
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
    } >x
}

#if PLAT_RO_XLAT_TABLES
#   include "fragments/base_xlat_table.ld.S"
#endif /* PLAT_RO_XLAT_TABLES */

SECTIONS {
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
    .plt . : {
        *(.plt)
        *(.iplt)
    } >x

    ASSERT(SIZEOF(.plt) == 0, "unexpected PLT found")

    . = .; /* Accommodate orphan and inserted sections */

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
    /*
     * When the executable segment and the read-only data segment would
     * otherwise be contiguous, we can separate the two to prevent any
     * non-executable data pages from being marked as executable.
     *
     * In this case, we align the read-only data sections to the next page
     * boundary.
     */
#if SEPARATE_CODE_AND_RODATA
    . = (ORIGIN(r) == ORIGIN(x) ? ALIGN(CONSTANT(MAXPAGESIZE)) : ORIGIN(r));
#else
    . = (ORIGIN(r) == ORIGIN(x) ? . : ORIGIN(r));
#endif /* SEPARATE_CODE_AND_RODATA */

    __R_START__ = .;
    __RODATA_START__ = .;

    /* Read-only data */
    .rodata . : {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    } >r

    /* Runtime service descriptor table */
    .rodata.rt_svc_descs . : {
        PROVIDE(__RT_SVC_DESCS_START__ = .);
        KEEP(*(rt_svc_descs))
        PROVIDE(__RT_SVC_DESCS_END__ = .);
    } >r

    /* Firmware Configuration Framework (FCONF) populator table */
    .rodata.fconf_populator . : {
        PROVIDE(__FCONF_POPULATOR_START__ = .);
        KEEP(*(.fconf_populator))
        PROVIDE(__FCONF_POPULATOR_END__ = .);
    } >r

    /* Performance Measurement Framework (PMF) service descriptor table */
    .rodata.pmf_svc_descs . : {
        PROVIDE(__PMF_SVC_DESCS_START__ = .);
        KEEP(*(pmf_svc_descs))
        PROVIDE(__PMF_SVC_DESCS_END__ = .);
    } >r

    /* Image Parsing library descriptor table */
    .rodata.img_parser_lib_descs . : {
        PROVIDE(__PARSER_LIB_DESCS_START__ = .);
        KEEP(*(.img_parser_lib_descs))
        PROVIDE(__PARSER_LIB_DESCS_END__ = .);
    } >r

    /* CPU operations table */
    .rodata.cpu_ops . : {
        PROVIDE(__CPU_OPS_START__ = .);
        KEEP(*(cpu_ops))
        PROVIDE(__CPU_OPS_END__ = .);
    } >r

#if SPMC_AT_EL3
    /* SPMC Logical Partition descriptor table */
    .rodata.el3_lp_descs . : {
        PROVIDE(__EL3_LP_DESCS_START__ = .);
        KEEP(*(el3_lp_descs))
        PROVIDE(__EL3_LP_DESCS_END__ = .);
    } >r
#endif /* SPMC_AT_EL3 */

    /*
     * Global Offset Table (GOT).
     *
     * The GOT is a table mapping position-independent symbols to their absolute
     * addresses in memory. This table would updated by the a dynamic linker if
     * one were available, but instead we do it ourselves.
     */
    .got . : {
        PROVIDE(__GOT_START__ = .);
        *(.got)
        PROVIDE(__GOT_END__ = .);
    } >r

#ifndef __PIE__
    ASSERT(SIZEOF(.got) == 0, "unexpected GOT found")
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
    .got.plt . : {
        *(.igot.plt)
    } >r

    ASSERT(SIZEOF(.got.plt) == 0, "unexpected GOT PLT entries found")

    /*
     * REL-type relocations.
     *
     * This table contains REL-type relocation information for symbols which
     * need to be relocated (usually by a dynamic linker, but by ourself in this
     * case).
     */
    .rel.dyn . : {
        PROVIDE(__REL_DYN_START__ = .);
        *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
        PROVIDE(__REL_DYN_END__ = .);
    } >r

#ifndef __PIE__
    ASSERT(SIZEOF(.rel.dyn) == 0, "unexpected relocations found")
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
    .rel.plt . : {
        *(.rel.plt)
    } >r

    ASSERT(SIZEOF(.rel.plt) == 0, "unexpected PLT relocations found")

    /*
     * RELA-type relocations.
     *
     * This table contains RELA-type relocation information for symbols which
     * need to be relocated (usually by a dynamic linker, but by ourself in this
     * case).
     */
    .rela.dyn . : {
        PROVIDE(__RELA_DYN_START__ = .);
        *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
        PROVIDE(__RELA_DYN_END__ = .);
    } >r

#ifndef __PIE__
    ASSERT(SIZEOF(.rela.dyn) == 0, "unexpected relocations found")
#endif

    /*
     * RELA-type relocations for PLT entries.
     *
     * This table contains RELA-type relocation information for symbols with PLT
     * entries which need to be relocated (usually by a dynamic linker, but by
     * ourself in this case).
     */
    .rela.plt . : {
        *(.rela.iplt)
    } >r

    ASSERT(SIZEOF(.rela.plt) == 0, "unexpected PLT relocations found")

    /*
     * We only support REL-type relocations in AArch32 builds, and RELA-type
     * relocations in AArch64 builds.
     */
#ifdef __aarch64__
    ASSERT(SIZEOF(.rel.dyn) == 0, "unexpected REL-type relocations found")

    PROVIDE(__RELA_START__ = __RELA_DYN_START__);
    PROVIDE(__RELA_END__ = __RELA_DYN_END__);
#else /* __aarch64__ */
    ASSERT(SIZEOF(.rela.dyn) == 0, "unexpected RELA-type relocations found")

    PROVIDE(__RELA_START__ = __REL_DYN_START__);
    PROVIDE(__RELA_END__ = __REL_DYN_END__);
#endif /* __aarch64__ */

    /*
     * Arm ELF32 exception-handling table.
     *
     * This table contains variable-size entries encoding, in a vendor- and
     * language-specific way, the actions required to propagate an exception
     * through a function.
     */
    .ARM.extab . : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >r

    /*
     * Arm ELF32 exception-handling index table.
     *
     * When exception handling is enabled, this table contains the offsets of
     * every entry in the exception-handling table given in the same order as
     * the addresses of their associated functions.
     */
    .ARM.exidx . : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >r

    ASSERT(SIZEOF(.ARM.extab) == 0,
        "unexpected exception handler table found")
    ASSERT(SIZEOF(.ARM.exidx) == 0,
        "unexpected exception handler index table found")

    . = .; /* Accommodate orphan and inserted sections */

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
    . = ALIGN(ORIGIN(w), CRT_DATA_ALIGN);

    __W_START__ = .;
    __DATA_START__ = .;

    /* Writable data */
    .data . : {
        *(.data .data.* .gnu.linkonce.d.*)
    } >w AT>r

    . = .; /* Accommodate orphan and inserted sections */
    . = ALIGN(CRT_DATA_ALIGN);

    __DATA_END__ = .;
    __DATA_SIZE__ = __DATA_END__ - __DATA_START__;

    __DATA_LOAD_START__ = LOADADDR(.data) - (ADDR(.data) - __DATA_START__);
    __DATA_LOAD_END__ = __DATA_LOAD_START__ + __DATA_SIZE__;
}

/*
 * Zero-initialized data sections.
 *
 * These sections contribute to the writable data segment of the bootloader
 * image, and zero-initialized by the C runtime.
 */
SECTIONS {
    __BSS_START__ = ALIGN(CRT_BSS_ALIGN);

    /* Zero-initialized data */
    .bss . (NOLOAD) : {
        /* Zero-initialized data */
        *(.bss .bss.* .gnu.linkonce.b.*)
    } >w
}

#if !PLAT_RO_XLAT_TABLES
#   include "fragments/base_xlat_table.ld.S"
#endif /* PLAT_RO_XLAT_TABLES */

SECTIONS {
    /* Bakery locks */
#if !USE_COHERENT_MEM
    .bss.bakery_locks . : {
        . = ALIGN(CACHE_WRITEBACK_GRANULE);

        __BAKERY_LOCK_START__ = .;
        __PERCPU_BAKERY_LOCK_START__ = .;

        *(bakery_lock)

        . = ALIGN(CACHE_WRITEBACK_GRANULE);

        __PERCPU_BAKERY_LOCK_END__ = .;
        __PERCPU_BAKERY_LOCK_SIZE__ =
            __PERCPU_BAKERY_LOCK_END__ - __PERCPU_BAKERY_LOCK_START__;

        . += (__PERCPU_BAKERY_LOCK_SIZE__ * (PLATFORM_CORE_COUNT - 1));

        __BAKERY_LOCK_END__ = .;
    } >w

#   ifdef PLAT_PERCPU_BAKERY_LOCK_SIZE
    ASSERT(
        (__PERCPU_BAKERY_LOCK_SIZE__ == 0) ||
            (__PERCPU_BAKERY_LOCK_SIZE__ == PLAT_PERCPU_BAKERY_LOCK_SIZE),
        "per-core bakery lock size does not meet requirements")
#   endif /* PLAT_PERCPU_BAKERY_LOCK_SIZE */
#endif /* !USE_COHERENT_MEM */

    /*
     * Performance Measurement Framework timestamps.
     *
     * The compiler will allocate enough memory for one core's timestamps, but
     * the remaining memory for other cores is allocated manually here.
     */
    .bss.pmf_timestamps . : {
        . = ALIGN(CACHE_WRITEBACK_GRANULE);

        __PMF_TIMESTAMP_START__ = .;

        KEEP(*(pmf_timestamp_array))

        . = ALIGN(CACHE_WRITEBACK_GRANULE);

        __PMF_PERCPU_TIMESTAMP_END__ = .;
        __PERCPU_TIMESTAMP_SIZE__ = ABSOLUTE(. - __PMF_TIMESTAMP_START__);

        . += (__PERCPU_TIMESTAMP_SIZE__ * (PLATFORM_CORE_COUNT - 1));

        __PMF_TIMESTAMP_END__ = .;
    } >w

    . = .; /* Accommodate orphan and inserted sections */
    . = ALIGN(CRT_BSS_ALIGN);

    __BSS_END__ = .;
}

/*
 * Uninitialized data sections.
 *
 * These sections contribute to the writable data segment of the bootloader
 * image, and are used as-is without initialization.
 */
SECTIONS {
    __NOINIT_START__ = .;

    /* Uninitialized data */
    .noinit . (NOLOAD) : {
        /* Uninitialized data */
        *(.noinit .noinit.* .gnu.linkonce.n.*)
    } >w

    /*
     * Per-core stack space.
     *
     * Core stacks have a strict alignment regime which differs between
     * execution states. Rather than enforce alignment here, we implicitly
     * respect the alignment of the stack sections.
     */
    .noinit.stacks . (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >w

    /*
     * Translation tables.
     */
    .noinit.xlat_tables . (NOLOAD) : {
        /* Translation tables */
        __XLAT_TABLE_START__ = .;
        *(xlat_table)
        __XLAT_TABLE_END__ = .;
    } >w

#if USE_COHERENT_MEM
    . = .;
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    __COHERENT_RAM_START__ = .;

    /*
     * Coherent memory region.
     *
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored in their own pages and are
     * not mixed with normal data. This is required to set up the correct memory
     * attributes for the coherent data page tables.
     */
    .noinit.coherent_ram . (NOLOAD) : {
        *(tzfw_coherent_mem)
    } >w

    __COHERENT_RAM_END_UNALIGNED__ = .;

    /*
     * Memory page(s) mapped to this section will be marked as device memory; no
     * other unexpected data must creep in. Ensure the rest of the current
     * memory page is unused.
     */
    . = ALIGN(PAGE_SIZE);

    __COHERENT_RAM_END__ = .;
#endif /* USE_COHERENT_MEM */

    . = .; /* Accommodate orphan and inserted sections */

    __NOINIT_END__ = .;
    __W_END__ = .;
}

/*
 * Metadata sections.
 *
 * These sections are not allocated, and consist purely of metadata. Most of
 * this metadata is intended for debugging and other tooling.
 */
SECTIONS {
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
