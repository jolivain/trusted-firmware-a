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
 * Before modifying this file, please take note of the following linker
 * behaviours that it takes advantage of:
 *
 * - Every section group starts with a `.start` section and ends with an `.end`
 *   section, through which the start and end alignments of the section group
 *   can be adjusted.
 *
 * - Orphan or inserted sections may be inserted before or after any section
 *   mentioned in this script, except before `.start` or after `.end` sections.
 *
 * - Output sections are generally declared in either ascending or descending
 *   order of alignment within section groups. This helps us to save on space
 *   that would have otherwise been used for padding. You can see the section
 *   alignment requirements of an image with `readelf`.
 *
 * - All sections are prefixed with the primary section with which they are
 *   associated (e.g. `.rodata`). There's no technical reason behind this - it
 *   just makes it easier to disseminate which sections have gone into which
 *   segments when inspecting the image with `readelf`.
 */

/*
 * The linker has an innate understanding of pages, and defines a couple of
 * constants for convenience. Make sure the preprocessor agrees with them.
 *
 * These constants are configured via the `-z {common,max}-page-size` options.
 */
ASSERT(CONSTANT(COMMONPAGESIZE) == PAGE_SIZE, "page size mismatch");
ASSERT(CONSTANT(MAXPAGESIZE) == PAGE_SIZE, "page size mismatch");

PHDRS {
    x PT_LOAD FLAGS(5 /* R_E */); /* `.text`-like data */
    r PT_LOAD FLAGS(4 /* R__ */); /* `.rodata`-like data */
    w PT_LOAD FLAGS(6 /* RW_ */); /* `.data`-like data */
    n PT_LOAD FLAGS(6 /* RW_ */); /* `.bss` or `.noinit`-like data */
}

SECTIONS {
    /*
     * Depending on the image configuration, some memory regions may alias. We
     * can use these macros to determine whether that is the case.
     *
     * If the read-only and writable data regions are the same, then writable
     * data is loaded in-place, rather than being explicitly loaded by the C
     * runtime.
     */
    HIDDEN(__SHARED_RX_REGION__ = ORIGIN(r) == ORIGIN(x));
    HIDDEN(__SHARED_WR_REGION__ = ORIGIN(w) == ORIGIN(r));
    HIDDEN(__SHARED_NW_REGION__ = ORIGIN(n) == ORIGIN(w));

    /*
     * Section group alignment requirements.
     *
     * Some images need to make adjustments to the default section alignment
     * requirements of this linker script, so allow individual images to
     * override these symbols as necessary.
     */

#if SEPARATE_CODE_AND_RODATA
    HIDDEN(__TEXT_ALIGNMENT__ = 0);
    HIDDEN(__TEXT_PADDING__ =
        __SHARED_RX_REGION__ ? CONSTANT(MAXPAGESIZE) : 0);
#else /* SEPARATE_CODE_AND_RODATA */
    HIDDEN(__TEXT_ALIGNMENT__ = 0);
    HIDDEN(__TEXT_PADDING__ = 0);
#endif /* SEPARATE_CODE_AND_RODATA */

    HIDDEN(__RODATA_ALIGNMENT__ = 0);
    HIDDEN(__RODATA_PADDING__ =
        __SHARED_WR_REGION__ ? CONSTANT(MAXPAGESIZE) : 0);

    HIDDEN(__DATA_ALIGNMENT__ = 0);
    HIDDEN(__DATA_PADDING__ = 0);

#if __aarch64__
    HIDDEN(__BSS_ALIGNMENT__ = 16); /* Required by our C runtime */
    HIDDEN(__BSS_PADDING__ = 16); /* Required by our C runtime */
#else
    HIDDEN(__BSS_ALIGNMENT__ = 8); /* Required by our C runtime */
    HIDDEN(__BSS_PADDING__ = 8); /* Required by our C runtime */
#endif /* __aarch64__ */

    HIDDEN(__NOINIT_ALIGNMENT__ = 0);
    HIDDEN(__NOINIT_PADDING__ = 0);

    /*
     * Section group sizes.
     */
    __TEXT_SIZE__ = __TEXT_END__ - __TEXT_START__;
    __RODATA_SIZE__ = __RODATA_END__ - __RODATA_START__;
    __DATA_SIZE__ = __DATA_END__ - __DATA_START__;
    __BSS_SIZE__ = __BSS_END__ - __BSS_START__;
    __NOINIT_SIZE__ = __NOINIT_END__ - __NOINIT_START__;
}

/*
 * Read-only executable sections.
 *
 * These sections contribute to the executable segment(s) of the bootloader
 * image, also known as text or code segments.
 */
SECTIONS {
    .text.start : {
        __TEXT_UNALIGNED_START__ = .;

        . = ALIGN(__TEXT_ALIGNMENT__);

        __TEXT_START__ = .;
    } >x AT>x :x

    /*
     * Bootloader entrypoints.
     *
     * In every bootloader image the entrypoint is placed at the beginning of
     * the image so that it aligns with the reset address or the image's load
     * address.
     */
    .text.entrypoint : {
        *(.text.asm.bl*_entrypoint)
    } >x AT>x :x

    /*
     * General read-only executable data.
     */
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
    } >x AT>x :x

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
    } >x AT>x :x

    ASSERT(SIZEOF(.text.plt) == 0,
        "Procedure Linkage Table found (lazy binding is not supported)")

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
    } >x AT>x :x

    .text.end : {
        __TEXT_UNALIGNED_END__ = .;

        . = ALIGN(__TEXT_PADDING__);

        __TEXT_END__ = .;
    } >x AT>x :x
}

/*
 * Read-only data sections.
 */
SECTIONS {
    .rodata.start : {
        __RODATA_UNALIGNED_START__ = .;

        . = ALIGN(__RODATA_ALIGNMENT__);

        __RODATA_START__ = .;
    } >r AT>r :r

    /*
     * General read-only data.
     */
    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    } >r AT>r :r

    /*
     * Runtime service descriptor table.
     *
     * This is a table of `rt_svc_desc` structures, which describe the set of
     * services managed by the Runtime Services Framework.
     */
    .rodata.rt_svc_descs : {
        __RT_SVC_DESCS_START__ = .;
        KEEP(*(.rt_svc_descs))
        __RT_SVC_DESCS_END__ = .;
    } >r AT>r :r

    /*
     * FCONF populator table.
     *
     * This is a table of `fconf_populator` structures, which describe the set
     * of FCONF populators executed by the Firmware Configuration Framework.
     */
    .rodata.fconf_populators : {
        __FCONF_POPULATOR_START__ = .;
        KEEP(*(.fconf_populator))
        __FCONF_POPULATOR_END__ = .;
    } >r AT>r :r

    /*
     * PMF service descriptor table.
     *
     * This is a table of `pmf_svc_desc` structures, which describe the set of
     * services managed by the Performance Measurement Framework.
     */
    .rodata.pmf_svc_descs : {
        __PMF_SVC_DESCS_START__ = .;
        KEEP(*(.pmf_svc_descs))
        __PMF_SVC_DESCS_END__ = .;
    } >r AT>r :r

    /*
     * Image Parsing Library descriptors table.
     *
     * This is a table of `img_parser_lib_desc_s` structures, which describe the
     * set of image parsing libraries managed by the Image Parsing Framework.
     */
    .rodata.img_parser_lib_descs : {
        __PARSER_LIB_DESCS_START__ = .;
        KEEP(*(.img_parser_lib_descs))
        __PARSER_LIB_DESCS_END__ = .;
    } >r AT>r :r

    /*
     * CPU Operations table.
     *
     * This is a table of `cpu_ops` structures, which describe operations
     * specific to each CPU in the system. These are part of the CPU-Specific
     * Operations Framework.
     */
    .rodata.cpu_ops : {
        __CPU_OPS_START__ = .;
        KEEP(*(.cpu_ops))
        __CPU_OPS_END__ = .;
    } >r AT>r :r

    /*
     * SPMC Logical Partition descriptors table.
     *
     * This is a table of `el3_lp_desc` structures, which describe the logical
     * partitions managed by the Secure Partition Management Core.
     */
    .rodata.el3_lp_descs : {
        __EL3_LP_DESCS_START__ = .;
        KEEP(*(.el3_lp_descs))
        __EL3_LP_DESCS_END__ = .;
    } >r AT>r :r

    /*
     * ELF dynamic section.
     *
     * We currently make no use of this, so we discard it.
     */
    /DISCARD/ /* .rodata.dynamic */ : { *(.dynamic) }

    /*
     * ELF hash table.
     *
     * We currently make no use of this, so we discard it.
     */
    /DISCARD/ /* .rodata.hash */ : { *(.hash) }

    /*
     * ELF dynamic symbol table.
     *
     * This table contains a subset of the symbols from the symbol table that
     * are needed to support dynamic linking.
     *
     * We currently make no use of this, so we discard it.
     */
    /DISCARD/ /* .rodata.dynsym */ : { *(.dynsym) }

    /*
     * ELF dynamic symbol string table.
     *
     * This table holds an array of C strings representing the names of each
     * dynamic symbol in the image file. The names are pointed to by the entries
     * in the dynamic symbol table.
     *
     * We currently make no use of this, so we discard it.
     */
    /DISCARD/ /* .rodata.dynstr */ : { *(.dynstr) }

    /*
     * GNU symbol versioning tables.
     *
     * This table describes the version of each dynamically-linked symbol in
     * the binary.
     *
     * This can be useful for shared libraries but, because we know we are
     * building an executable and there is nobody trying to link with us, we can
     * safely discard them.
     */
    /DISCARD/ /* .rodata.gnu.version */ : { *(.gnu.version) }
    /DISCARD/ /* .rodata.gnu.version_d */ : { *(.gnu.version_d) }
    /DISCARD/ /* .rodata.gnu.version_r */ : { *(.gnu.version_r) }

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
    } >r AT>r :r

#ifndef __PIE__
    ASSERT(SIZEOF(.rodata.got) == 0,
        "Global Offset Table found (PIE is not enabled for this image)")
#endif

    /*
     * Global Offset Table (GOT) for PLT entries.
     *
     * This table follows the same scheme as the section above, except it
     * contains entries which are patched by the trampolines in the Procedure
     * Linkage Table (PLT).
     *
     * Because we don't support lazy binding there is no PLT to patch this GOT,
     * so it should be empty.
     */
    .rodata.got.plt : {
        *(.got.plt .igot.plt)
    } >r AT>r :r

    ASSERT((SIZEOF(.rodata.got.plt) == 0) || (SIZEOF(.rodata.got.plt) == 0x18),
        "Global Offset Table has PLT entries (lazy binding is not supported)")

    /*
     * RELA-type relocations.
     *
     * This table contains RELA-type relocation information for symbols which
     * need to be relocated (usually by a dynamic linker, but by ourself in this
     * case).
     */
    .rodata.rela.dyn : {
        __RELA_DYN_START__ = .;
        *(.rela.*)
        __RELA_DYN_END__ = .;
    } >r AT>r :r

    /*
     * REL-type relocations.
     *
     * This table contains REL-type relocation information for symbols which
     * need to be relocated (usually by a dynamic linker, but by ourself in this
     * case).
     */
    .rodata.rel.dyn : {
        __REL_DYN_START__ = .;
        *(.rel.*)
        __REL_DYN_END__ = .;
    } >r AT>r :r

#ifndef __PIE__
    ASSERT(SIZEOF(.rodata.rel.dyn) == 0,
        "REL-type relocations found (PIE is not enabled for this image)")
    ASSERT(SIZEOF(.rodata.rela.dyn) == 0,
        "RELA-type relocations found (PIE is not enabled for this image)")
#endif

#ifdef __aarch64__
    ASSERT(SIZEOF(.rodata.rel.dyn) == 0,
        "REL-type relocations found (expected RELA-type relocations)")

    __RELA_START__ = __RELA_DYN_START__;
    __RELA_END__ = __RELA_DYN_END__;
#else /* __aarch64__ */
    ASSERT(SIZEOF(.rodata.rela.dyn) == 0,
        "RELA-type relocations found (expected REL-type relocations)")

    __RELA_START__ = __REL_DYN_START__;
    __RELA_END__ = __REL_DYN_END__;
#endif /* __aarch64__ */

    /*
     * REL-type relocations for PLT entries.
     *
     * This table contains REL-type relocation information for Global Offset
     * Table (GOT) entries which are patched by the trampolines in the Procedure
     * Linkage Table (PLT).
     *
     * Because we don't support lazy binding there is no PLT to read this
     * relocation table.
     */
    .rodata.rel.plt : {
        *(.rel.plt) *(.rel.iplt)
    } >r AT>r :r

    /*
     * RELA-type relocations for PLT entries.
     *
     * This table contains RELA-type relocation information for Global Offset
     * Table (GOT) entries which are patched by the trampolines in the Procedure
     * Linkage Table (PLT).
     *
     * Because we don't support lazy binding there is no PLT to read this
     * relocation table.
     */
    .rodata.rela.plt : {
        *(.rela.plt) *(.rela.iplt)
    } >r AT>r :r

    ASSERT(SIZEOF(.rodata.rel.plt) == 0,
        "REL-type PLT relocations found (lazy binding is not supported)")
    ASSERT(SIZEOF(.rodata.rela.plt) == 0,
        "RELA-type PLT relocations found (lazy binding is not supported)")

    /*
     * Arm ELF32 exception-handling table.
     *
     * This table contains variable-size entries encoding, in a vendor- and
     * language-specific way, the actions required to propagate an exception
     * through a function.
     *
     * We don't support stack unwinding, so exceptions are unsupported.
     */
    /DISCARD/ /* .rodata.ARM.extab */ : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    }

    /*
     * Arm ELF32 exception-handling index table.
     *
     * When exception handling is enabled, this table contains the offsets of
     * every entry in the exception-handling table given in the same order as
     * the addresses of their associated functions.
     *
     * LLD doesn't give us any way to suppress generation of this section and
     * it reports a size larger than it really is, so just discard it.
     */
    /DISCARD/ /* .rodata.ARM.exidx */ : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    }

    .rodata.end : {
        __RODATA_UNALIGNED_END__ = .;

        . = __SHARED_WR_REGION__ ? . : ALIGN(__DATA_ALIGNMENT__);

        __DATA_LOAD_START__ = __SHARED_WR_REGION__ ? __DATA_START__ : .;

        . = __SHARED_WR_REGION__ ? . :
            ALIGN(. + __DATA_SIZE__, __DATA_ALIGNMENT__);

        __DATA_LOAD_END__ = __SHARED_WR_REGION__ ? __DATA_END__ : .;

        . = ALIGN(__RODATA_PADDING__);

        __RODATA_END__ = .;
    } >r AT>r :r
}

#if PLAT_RO_XLAT_TABLES
#   include <bootloader/base_xlat_table.ld.S>
#endif /* PLAT_RO_XLAT_TABLES */

/*
 * Writable data sections.
 *
 * These sections contribute to the writable data segment(s) of the bootloader
 * image.
 */
SECTIONS {
    .data.start : {
        __DATA_UNALIGNED_START__ = .;

        . = ALIGN(__DATA_ALIGNMENT__);

        __DATA_START__ = .;
    } >w AT>r :w

    /*
     * General writable data.
     */
    .data : {
        *(.data .data.* .gnu.linkonce.d.*)
    } >w AT>r :w

    .data.end : {
        __DATA_UNALIGNED_END__ = .;

        . = ALIGN(__DATA_PADDING__);

        __DATA_END__ = .;
    } >w AT>r :w
}

/*
 * Per-core stack space section.
 *
 * This section is placed between the writable data section group and the
 * zero-initialized data section group because that has historically been where
 * it sat, and it's unclear whether there is anything that relies on that.
 */
SECTIONS {
    .noinit.stacks (NOLOAD) : {
        __STACKS_START__ = .;
        *(.tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >n AT>n :n
}

/*
 * Zero-initialized data sections.
 *
 * These sections contribute to the writable data segment of the bootloader
 * image, and zero-initialized by the C runtime.
 */
SECTIONS {
    .bss.start (NOLOAD) : {
        __BSS_UNALIGNED_START__ = .;

        . = ALIGN(__BSS_ALIGNMENT__);

        __BSS_START__ = .;
    } >n AT>n :n

    /*
     * General zero-initialized data.
     */
    .bss (NOLOAD) : {
        *(.dynbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
    } >n AT>n :n

    /*
     * Bakery locks.
     *
     * When coherent memory is disabled, the bakery locks can be placed in
     * normal memory.
     *
     * The compiler only allocates enough memory for one core's timestamps, so
     * the remaining memory for the other cores is allocated manually here.
     */
#if !USE_COHERENT_MEM
    .bss.bakery_locks (NOLOAD) : ALIGN(CACHE_WRITEBACK_GRANULE) {
        __BAKERY_LOCK_START__ = .;
        __PERCPU_BAKERY_LOCK_START__ = .;

        *(.bakery_lock)

        . = ALIGN(CACHE_WRITEBACK_GRANULE);

        __PERCPU_BAKERY_LOCK_END__ = .;
        __PERCPU_BAKERY_LOCK_SIZE__ =
            ABSOLUTE(__PERCPU_BAKERY_LOCK_END__ - __PERCPU_BAKERY_LOCK_START__);

        . += __PERCPU_BAKERY_LOCK_SIZE__ * (PLATFORM_CORE_COUNT - 1);

        __BAKERY_LOCK_END__ = .;
    } >n AT>n :n

#   ifdef PLAT_PERCPU_BAKERY_LOCK_SIZE
    ASSERT(
        (__PERCPU_BAKERY_LOCK_SIZE__ == 0) ||
            (__PERCPU_BAKERY_LOCK_SIZE__ == PLAT_PERCPU_BAKERY_LOCK_SIZE),
        "per-core bakery lock size does not meet requirements")
#   endif /* PLAT_PERCPU_BAKERY_LOCK_SIZE */
#endif /* !USE_COHERENT_MEM */

    /*
     * PMF timestamps.
     *
     * The compiler only allocates enough memory for one core's timestamps, so
     * the remaining memory for the other cores is allocated manually here.
     */
    .bss.pmf_timestamps (NOLOAD) : ALIGN(CACHE_WRITEBACK_GRANULE) {
        __PMF_TIMESTAMP_START__ = .;

        KEEP(*(.pmf_timestamp_array))

        . = ALIGN(CACHE_WRITEBACK_GRANULE);

        __PMF_PERCPU_TIMESTAMP_END__ = .;
        __PERCPU_TIMESTAMP_SIZE__ =
            ABSOLUTE(__PMF_PERCPU_TIMESTAMP_END__ - __PMF_TIMESTAMP_START__);

        . += (__PERCPU_TIMESTAMP_SIZE__ * (PLATFORM_CORE_COUNT - 1));

        __PMF_TIMESTAMP_END__ = .;
    } >n AT>n :n
}

#if !PLAT_RO_XLAT_TABLES
#   include <bootloader/base_xlat_table.ld.S>
#endif /* PLAT_RO_XLAT_TABLES */

SECTIONS {
    .bss.end (NOLOAD) : {
        __BSS_UNALIGNED_END__ = .;

        . = ALIGN(__BSS_PADDING__);

        __BSS_END__ = .;
    } >n AT>n :n
}

/*
 * Uninitialized data sections.
 *
 * These sections contribute to the writable data segment of the bootloader
 * image, and are used as-is without initialization.
 *
 * Keep in mind that it is undefined behaviour for a C program to read from
 * objects placed in this memory without first explicitly initializing them, or
 * otherwise marking them as `volatile`.
 */
SECTIONS {
    .noinit.start (NOLOAD) : {
        __NOINIT_UNALIGNED_START__ = .;

        . = ALIGN(__NOINIT_ALIGNMENT__);

        __NOINIT_START__ = .;
    } >n AT>n :n

    /*
     * General uninitialized data.
     */
    .noinit (NOLOAD) : {
        *(.noinit .noinit.* .gnu.linkonce.n.*)
    } >n AT>n :n

    /*
     * Translation tables.
     *
     * The translation tables configure the translation regime for the MMU.
     * We place them first here because they have a large alignment requirement,
     * which saves us some space which otherwise would have been padded.
     *
     * These are placed in the uninitialized data section to avoid being zeroed
     * by the runtime, as the translation tables library zeroes them instead.
     */
    .noinit.xlat_tables (NOLOAD) : {
        __XLAT_TABLE_START__ = .;
        *(.xlat_table)
        __XLAT_TABLE_END__ = .;
    } >n AT>n :n

    .noinit.end (NOLOAD) : {
        __NOINIT_UNALIGNED_END__ = .;

        . = ALIGN(__NOINIT_PADDING__);

        __NOINIT_END__ = .;
    } >n AT>n :n
}

SECTIONS {
    .x.end : {
        __X_REGION_START__ = ORIGIN(x);
        __X_REGION_LENGTH__ = LENGTH(x);
        __X_REGION_END__ = .;
    } >x AT>x :x

    .r.end : {
        __R_REGION_START__ = ORIGIN(r);
        __R_REGION_LENGTH__ = LENGTH(r);
        __R_REGION_END__ = .;
    } >r AT>r :r

    .w.end : {
        __W_REGION_START__ = ORIGIN(w);
        __W_REGION_LENGTH__ = LENGTH(w);
        __W_REGION_END__ = .;
    } >w AT>w :w

    .n.end (NOLOAD) : {
        __N_REGION_START__ = ORIGIN(n);
        __N_REGION_LENGTH__ = LENGTH(n);
        __N_REGION_END__ = .;
    } >n AT>n :n
}

/*
 * Metadata sections.
 *
 * These sections are not allocated, and consist purely of metadata. Most of
 * this metadata is intended for debugging and other tooling.
 */
SECTIONS {
    /*
     * ELF symbol table.
     *
     * The symbol table holds information needed to locate and relocate a
     * program's symbolic definitions and symbolic references.
     */
    .symtab 0 : { *(.symtab) }

    /*
     * ELF symbol string table.
     *
     * This table holds an array of C strings representing the names of each
     * symbol in the image file. The names are pointed to by the entries in the
     * symbol table.
     */
    .strtab 0 : { *(.strtab) }

    /*
     * ELF section header string table.
     *
     * Similar to the string table, this table contains the names of the
     * sections in the image file.
     */
    .shstrtab 0 : { *(.shstrtab) }

    /*
     * DWARF debug information.
     */
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_aranges 0 : { *(.debug_aranges) }
    .debug_frame 0 : { *(.debug_frame) }
    .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_line 0 : { *(.debug_line .debug_line.* .debug_line_end) }
    .debug_loc 0 : { *(.debug_loc) }
    .debug_macinfo 0 : { *(.debug_macinfo) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
    .debug_ranges 0 : { *(.debug_ranges) }
    .debug_str 0 : { *(.debug_str) }

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

    /*
     * GNU executable stack note.
     *
     * The presence of this section indicates to the linker that the stack is
     * not intended to be executable.
     */
    /DISCARD/ : {
        *(.note.GNU-stack)
    }
}
