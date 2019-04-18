/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.global	pauth_load_bl_apiakey
	.global	pauth_load_bl_apiakey_enable

/* -----------------------------------------------------
 * The following functions strictly follow the AArch64
 * PCS to use x9-x17 (temporary caller-saved registers)
 * to load the APIA key used by the firmware.
 * -----------------------------------------------------
 */
func pauth_load_bl_apiakey
	/* Load instruction key A used by the Trusted Firmware */
	adrp	x11, plat_apiakey
	add	x11, x11, :lo12:plat_apiakey
	ldp	x9, x10, [x11]

	msr	APIAKeyLo_EL1, x9
	msr	APIAKeyHi_EL1, x10
	isb

	ret
endfunc pauth_load_bl_apiakey

func pauth_load_bl_apiakey_enable
	/* Load instruction key A used by the Trusted Firmware */
	adrp	x11, plat_apiakey
	add	x11, x11, :lo12:plat_apiakey
	ldp	x9, x10, [x11]

	msr	APIAKeyLo_EL1, x9
	msr	APIAKeyHi_EL1, x10

	/* Enable pointer authentication */
#ifdef IMAGE_BL31
	mrs	x0, sctlr_el3
	orr	x0, x0, #SCTLR_EnIA_BIT
	msr	sctlr_el3, x0

#elif defined(IMAGE_BL32)
	mrs	x0, sctlr_el1
	orr	x0, x0, #SCTLR_EnIA_BIT
	msr	sctlr_el1, x0
#endif
	isb

	ret
endfunc pauth_load_bl_apiakey_enable

