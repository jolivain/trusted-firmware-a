/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_RECLAIM_INIT_LD_S
#define ARM_RECLAIM_INIT_LD_S

SECTIONS
{
        .init __STACKS_START__ : {
            . = . + PLATFORM_STACK_SIZE;
            . = ALIGN(PAGE_SIZE);
            __INIT_CODE_START__ = .;
	    *(*text.init*);
            __INIT_CODE_END__ = .;
            INIT_CODE_END_ALIGNED = ALIGN(PAGE_SIZE);
        } >RAM

#ifdef BL31_PROGBITS_LIMIT
    ASSERT(__INIT_CODE_END__ <= BL31_PROGBITS_LIMIT,
            "BL31 init has exceeded progbits limit.")
#endif

    /* Check that there is more than 1 core - required for RECLAIM_INIT_CODE. */
    ASSERT(PLATFORM_CORE_COUNT > 1,
	"RECLAIM_INIT_CODE feature requires multiple cores.")
    /* Check that the init section is within stack space, when page-aligned. */
    ASSERT(INIT_CODE_END_ALIGNED <= __STACKS_END__,
	"Init code ends past the end of the stack - PLATFORM_STACK_SIZE too small.")

}

#endif /* ARM_RECLAIM_INIT_LD_S */
