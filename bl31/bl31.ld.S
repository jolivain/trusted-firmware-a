/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>

ENTRY(bl31_entrypoint)

MEMORY {
    ram (rwx): ORIGIN = BL31_BASE, LENGTH = BL31_LIMIT - BL31_BASE

#if SEPARATE_NOBITS_REGION
    nobits (w!a): ORIGIN = BL31_NOBITS_BASE, LENGTH = BL31_NOBITS_LIMIT - BL31_NOBITS_BASE
#endif /* SEPARATE_NOBITS_REGION */
}

REGION_ALIAS("x", ram)
REGION_ALIAS("r", ram)
REGION_ALIAS("w", ram)

#if SEPARATE_NOBITS_REGION
REGION_ALIAS("n", nobits)
#else /* SEPARATE_NOBITS_REGION */
REGION_ALIAS("n", ram)
#endif /* SEPARATE_NOBITS_REGION */

ASSERT(BL31_BASE == ALIGN(BL31_BASE, CONSTANT(MAXPAGESIZE)),
    "BL31 address not aligned on a page boundary")

#if SEPARATE_NOBITS_REGION
ASSERT(BL31_NOBITS_BASE == ALIGN(BL31_NOBITS_BASE, CONSTANT(MAXPAGESIZE)),
    "BL31 NOBITS address not aligned on a page boundary")
#endif /* SEPARATE_NOBITS_REGION */

SECTIONS {
    __RAM_REGION_LENGTH__ = LENGTH(ram);
    __RAM_REGION_START__ = ORIGIN(ram);
    __RAM_REGION_END__ = __N_END__;

#if SEPARATE_NOBITS_REGION
    __RAM_REGION_END__ = __W_END__;
#endif /* SEPARATE_NOBITS_REGION */

#if SEPARATE_NOBITS_REGION
    __NOBITS_REGION_LENGTH__ = LENGTH(nobits);
    __NOBITS_REGION_START__ = ORIGIN(nobits);
    __NOBITS_REGION_END__ = __N_END__;
#endif /* SEPARATE_NOBITS_REGION */

    __BL31_START__ = __RAM_REGION_START__;
    __BL31_END__ = __RAM_REGION_END__;

    __RO_START__ = __X_START__;
    __RO_END__ = __R_END__;

    __RW_START__ = __W_START__;
    __RW_END__ = __N_END__;

    HIDDEN(__RODATA_PADDING__ = CONSTANT(MAXPAGESIZE));
}

INSERT BEFORE .text.start;

SECTIONS {
    /*
     * Platform-specific read-only data.
     */
    .rodata.platform : {
#if PLAT_EXTRA_RODATA_INCLUDES
#   include <plat.ld.rodata.inc>
#endif /* PLAT_EXTRA_RODATA_INCLUDES */
    } >r AT>r :rodata

    /*
     * Pub-sub events.
     */
    .rodata.pubsub_events : {
#include <lib/el3_runtime/pubsub_events.h>
    } >r AT>r :rodata
}

INSERT AFTER .rodata;

#ifdef PLAT_EXTRA_LD_SCRIPT
#   include <plat.ld.S>
#endif /* PLAT_EXTRA_LD_SCRIPT */

#if SPM_MM
#   if SPM_MM_RELOCATE_SHIM_EXCEPTIONS
#       include <bootloader/spm_shim_exceptions.config.ld.S>
#   else /* SPM_MM_RELOCATE_SHIM_EXCEPTIONS */
#       define SPM_MM_SHIM_EXCEPTIONS_REGION x
#       define SPM_MM_SHIM_EXCEPTIONS_PHDR text
#   endif /* SPM_MM_RELOCATE_SHIM_EXCEPTIONS */

SECTIONS {
    /*
     * Exception vectors of the SPM shim layer. They must be aligned to a 2K
     * address but we need to place them in a separate page so that we can set
     * individual permissions on them, so the actual alignment needed is the
     * page size.
     *
     * There's no need to include this into the RO section of BL31 because it
     * doesn't need to be accessed by BL31.
     */
    .text.spm_shim_exceptions : ALIGN(CONSTANT(MAXPAGESIZE)) {
        __SPM_SHIM_EXCEPTIONS_START__ = .;

        *(.spm_shim_exceptions)

        . = ALIGN(CONSTANT(MAXPAGESIZE));

        __SPM_SHIM_EXCEPTIONS_END__ = .;
    } >SPM_MM_SHIM_EXCEPTIONS_REGION AT>x :SPM_MM_SHIM_EXCEPTIONS_PHDR

    PROVIDE(__SPM_SHIM_EXCEPTIONS_LMA__ = LOADADDR(.text.spm_shim_exceptions));
}
#endif /* SPM_MM */

INSERT BEFORE .text.end;

#if RECLAIM_INIT_CODE
#   include <bootloader/reclaim_init_code.ld.S>
#endif /* RECLAIM_INIT_CODE */
