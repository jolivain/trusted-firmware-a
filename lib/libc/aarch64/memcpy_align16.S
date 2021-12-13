/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	memcpy_align16
/* --------------------------------------------------------------------------
 * void memcpy_align16(void *dest, const void *src, unsigned long length)
 *
 * Copy <length> bytes from memory area <src> to memory area <dest>.
 * The memory areas should not overlap.
 * Data is copied 16 bytes at a time using ldp/stp, so both src and dst should
 * be 8 byte aligned (see memcpy_align.c). If length is not a mulitple of 16,
 * the remaining bytes are copied over in decreasing powers of two.
 * --------------------------------------------------------------------------
 */
func memcpy_align16
	/* Skip m_loop16 if len < 16 */
	cmp	x2, #16
	b.lt	m_cpy8

	/* x6 = floor(len/16) */
	lsr	x6, x2, #4
m_loop16:
	ldp	x3, x4, [x1], #16
	stp	x3, x4, [x0], #16
	subs	x6, x6, #1
	bne	m_loop16

/* len is max 15 (1111) */
m_cpy8:
	tbz	x2, #3, m_cpy4
	ldr x3, [x1], #8
	str x3, [x0], #8
m_cpy4:
	tbz	x2, #2, m_cpy2
	ldr w3, [x1], #4
	str	w3, [x0], #4
m_cpy2:
	tbz	x2, #1, m_cpy1
	ldrh	w3, [x1], #2
	strh	w3, [x0], #2
m_cpy1:
	tbz	x2, #0, end
	ldrb	w3, [x1], #1
	strb	w3, [x0], #1
end:
	ret
endfunc memcpy_align16
