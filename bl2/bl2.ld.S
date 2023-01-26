/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>

ENTRY(bl2_entrypoint)

MEMORY {
    ram (rwx): ORIGIN = BL2_BASE, LENGTH = BL2_LIMIT - BL2_BASE
}

REGION_ALIAS("x", ram)
REGION_ALIAS("r", ram)
REGION_ALIAS("w", ram)
REGION_ALIAS("n", ram)

ASSERT(BL2_BASE == ALIGN(BL2_BASE, CONSTANT(MAXPAGESIZE)),
    "BL2 address not aligned on a page boundary")

SECTIONS {
    __RAM_REGION_START__ = ORIGIN(ram); /* For metrics collection */
    __RAM_REGION_END__ = __N_END__; /* For metrics collection */
    __RAM_REGION_LENGTH__ = LENGTH(ram); /* For metrics collection */

    __BL2_START__ = __RAM_REGION_START__; /* For backwards compatibility */
    __BL2_END__ = __RAM_REGION_END__; /* For backwards compatibility */

    __RO_START__ = __X_START__; /* For backwards compatibility */
    __RO_END__ = __R_END__; /* For backwards compatibility */

    __RW_START__ = __W_START__; /* For backwards compatibility */
    __RW_END__ = __N_END__; /* For backwards compatibility */

    __RODATA_PADDING__ = CONSTANT(MAXPAGESIZE);
}

INSERT BEFORE .text.start;
