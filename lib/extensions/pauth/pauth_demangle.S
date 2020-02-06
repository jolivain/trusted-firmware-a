/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.global	demangle_address

/* ----------------------------------------------------------------------------
 * Strip the Pointer Authentication Code (PAC) from the address to retrieve the
 * original one.
 *
 * The PAC field is stored on the high bits of the address and defined as:
 * - PAC field = Xn[54:bottom_PAC_bit], when address tagging is used.
 * - PAC field = Xn[63:56, 54:bottom_PAC_bit], without address tagging.
 *
 * With bottom_PAC_bit = 64 - TCR_ELx.TnSZ
 *
 * uintptr_t demangle_address(uintptr_t addr, unsigned int el)
 * ----------------------------------------------------------------------------
 */
func demangle_address
	/* ----------------------------------------------------------------
	 * Different virtual address space size can be defined for each EL.
	 * Ensure that we use the proper one by reading the corresponding
	 * TCR_ELx register.
	 * ----------------------------------------------------------------
	 */
	cmp	x1, #MODE_EL2
	b.lt	1f		/* EL1 */
	b.eq	2f		/* EL2 */
	mrs	x2, sctlr_el3	/* EL3 */
	mrs	x1, tcr_el3

	/* ----------------------------------------------------------------
	 * Check if pointer authentication is enabled at the specified EL.
	 * If it isn't, we can then skip stripping a PAC code.
	 * ----------------------------------------------------------------
	 */
3:	tst	x2, #(SCTLR_EnIA_BIT | SCTLR_EnIB_BIT)
	b.eq	4f
	and	x1, x1, #0x3F	/* T0SZ = TCR_ELx[5:0] */
	sub	x1, x1, #64
	neg	x1, x1		/* bottom_pac_bit = 64 - T0SZ */
	mov	x2, #-1
	lsl	x2, x2, x1
	bic	x0, x0, x2
4:	ret

	/* EL1 */
1:	mrs	x2, sctlr_el1
	mrs	x1, tcr_el1
	b	3b

	/* EL2 */
2:	mrs	x2, sctlr_el2
	mrs	x1, tcr_el2
	b	3b

endfunc	demangle_address
