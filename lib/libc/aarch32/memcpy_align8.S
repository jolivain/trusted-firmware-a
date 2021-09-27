/*
 * Copyright (c) 2016-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>

	.globl	memcpy_align8
/* --------------------------------------------------------------------------
 * void memcpy_align8(void *dest, const void *src, unsigned int length)
 *
 * Copy length bytes from memory area src to memory area dest.
 * The memory areas should not overlap.
 * Destination and source addresses must be 8-byte aligned.
 * --------------------------------------------------------------------------
 */

func memcpy_align8
/* copy 8 bytes at a time */
m_loop8:
	cmp	r2, #8
	blo	m_loop1
	ldrd	r4, r5, [r1], #8
	strd	r4, r5, [r0], #8
	subs	r2, r2, #8
	bne	m_loop8
	bx	lr

/* copy byte per byte */
m_loop1:
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
	subs	r2, r2, #1
	bne	m_loop1
	bx	lr
endfunc memcpy_align8
