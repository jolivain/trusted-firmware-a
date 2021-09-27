/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>

	.globl	memcpy_align16
/* --------------------------------------------------------------------------
 * void memcpy_align16(void *dest, const void *src, unsigned long length)
 *
 * Copy <length> bytes from memory area <src> to memory area <dest>.
 * The memory areas should not overlap.
 * Data is copied 16 bytes at a time using ldp/stp, so both src and dst should
 * be 16 byte aligned (see memcpy_align.c). If length is not a mulitple of 16,
 * any remaining bytes will be copied one at a time as in the original c
 * implementation (memcpy.c).
 * --------------------------------------------------------------------------
 */
func memcpy_align16
/* copy 16 bytes at a time */
m_loop16:
	cmp	x2, #16
	b.lo	m_loop1
	ldp	x3, x4, [x1], #16
	stp	x3, x4, [x0], #16
	sub	x2, x2, #16
	bne	m_loop16
	ret

/* copy byte per byte */
m_loop1:
	ldrb	w3, [x1], #1
	strb	w3, [x0], #1
	subs	x2, x2, #1
	b.ne	m_loop1
	ret
endfunc memcpy_align16
