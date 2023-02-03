/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if SPM_MM_RELOCATE_SHIM_EXCEPTIONS
#   include <bootloader/spm_shim_exceptions.config.ld.S>
#else /* SPM_MM_RELOCATE_SHIM_EXCEPTIONS */
#   define SPM_MM_SHIM_EXCEPTIONS_REGION x
#   define SPM_MM_SHIM_EXCEPTIONS_PHDR x
#endif /* SPM_MM_RELOCATE_SHIM_EXCEPTIONS */

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

INSERT BEFORE .text.end;
