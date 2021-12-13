/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.syntax	unified
	.globl	memcpy_align8
/* --------------------------------------------------------------------------
 * void memcpy_align8(void *dest, const void *src, unsigned int length)
 *
 * Copy <length> bytes from memory area <src> to memory area <dest>.
 * The memory areas should not overlap.
 * Data is copied 8 bytes at a time using ldrd/strd, so both src and dst should
 * be 8 byte aligned (see memcpy_align.c). If length is not a mulitple of 8,
 * any remaining bytes will be copied one at a time as in the original C
 * implementation (memcpy.c).
* --------------------------------------------------------------------------
 */

func memcpy_align8
	push	{r3-r7}
	and	r7, r2, #7

	/* Skip m_loop8 if len < 8 */
	cmp	r2, #8
	blt	m_loop1

	lsr	r6, r2, #3
/* copy 8 bytes at a time */
m_loop8:
	ldrd	r4, r5, [r1], #8
	strd	r4, r5, [r0], #8
	subs	r6, r6, #1
	bne	m_loop8

	cmp	r7, #0
	beq	end
/* copy byte per byte */
m_loop1:
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
	subs	r7, r7, #1
	bne	m_loop1
end:
	pop	{r3-r7}
	bx	lr
endfunc memcpy_align8
