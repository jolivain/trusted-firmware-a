/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bootloader/reclaim_init_code.config.ld.S>

PHDRS {
    reclaim PT_LOAD FLAGS(5 /* R_E */);
}

SECTIONS {
    HIDDEN(__RECLAIM_INIT_CODE_START__ = ADDR(RECLAIM_INIT_CODE_SECTION));
    HIDDEN(__RECLAIM_INIT_CODE_START_INTERNAL__ = __RECLAIM_INIT_CODE_START__ +
        RECLAIM_INIT_CODE_SECTION_OFFSET);

    HIDDEN(__RECLAIM_INIT_CODE_PAGE_START__ =
        ALIGN(__RECLAIM_INIT_CODE_START_INTERNAL__, CONSTANT(MAXPAGESIZE)));
    HIDDEN(__RECLAIM_INIT_CODE_PAGE_END__ =
        ALIGN(__INIT_CODE_END__, CONSTANT(MAXPAGESIZE)));

    RECLAIM_INIT_CODE_SECTION.padding (NOLOAD) : {
        __X__ = .;
        __Y__ = __RECLAIM_INIT_CODE_PAGE_END__;

        . = ABSOLUTE((. > __RECLAIM_INIT_CODE_PAGE_END__) ?
            . : __RECLAIM_INIT_CODE_PAGE_END__);
    } >RECLAIM_INIT_CODE_SECTION_REGION AT>RECLAIM_INIT_CODE_SECTION_REGION
        :RECLAIM_INIT_CODE_SECTION_PHDR

    ASSERT(ADDR(RECLAIM_INIT_CODE_SECTION.padding) ==
            ADDR(RECLAIM_INIT_CODE_SECTION) + SIZEOF(RECLAIM_INIT_CODE_SECTION),
        "reclaimable section padding is not adjacent to the root section")

    .text.init __RECLAIM_INIT_CODE_START__ : AT(__RECLAIM_INIT_CODE_START__) {
        . = ABSOLUTE(__RECLAIM_INIT_CODE_PAGE_START__);

        __INIT_CODE_START__ = .;
        *(.text.init.*);
        __INIT_CODE_END__ = .;
    } :reclaim
}

#ifdef BL31_PROGBITS_LIMIT
ASSERT(__INIT_CODE_END__ <= BL31_PROGBITS_LIMIT,
    "BL31 reclaimable initialization code has exceeded PROGBITS limit")
#endif

INSERT AFTER RECLAIM_INIT_CODE_SECTION;
