/*
 * Copyright (c) 2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <aarch64_generic.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func aarch64_generic_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc aarch64_generic_disable_dcache

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A35.
	 * Clobbers: x0
	 * -------------------------------------------------
	 */
func aarch64_generic_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	ret	x19
endfunc aarch64_generic_reset_func

func aarch64_generic_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	aarch64_generic_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	ret	x18
endfunc aarch64_generic_core_pwr_dwn

func aarch64_generic_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	aarch64_generic_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Disable the optional ACP.
	 * ---------------------------------------------
	 */
	bl	plat_disable_acp

	/* ---------------------------------------------
	 * Flush L2 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level2

	ret	x18

endfunc aarch64_generic_cluster_pwr_dwn

/* ---------------------------------------------
 * Unimplemented functions.
 * ---------------------------------------------
 */
.equ	aarch64_generic_errata_report,		0
.equ	aarch64_generic_cpu_reg_dump,		0

declare_cpu_ops aarch64_generic, AARCH64_GENERIC_MIDR, \
	aarch64_generic_reset_func, \
	aarch64_generic_core_pwr_dwn, \
	aarch64_generic_cluster_pwr_dwn
