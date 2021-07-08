/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>

	.globl	sme_rdvl_1

/*
 * uint32_t sme_rdvl_1(void);
 *
 * Run rdvl instruction with imm #1.
 */
func sme_rdvl_1
	rdvl	x0, #1
	ret
endfunc sme_rdvl_1
