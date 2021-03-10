/*
 * Copyright (c) 2014-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>
#include <qemu_max.h>

func max_generic_core_pwr_dwn
	/* ---------------------------------------------
	 * Disable the Data Cache.
	 * ---------------------------------------------
	 */
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb

	/* ---------------------------------------------
	 * Flush L1 cache to L2.
	 * ---------------------------------------------
	 */
	bl	dcsw_op_level1
	mov	x30, x18

	/* ---------------------------------------------
	 * Flush L2 cache to L3.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	b	dcsw_op_level2
endfunc max_generic_core_pwr_dwn

func max_generic_cluster_pwr_dwn
	/* ---------------------------------------------
	 * Disable the Data Cache.
	 * ---------------------------------------------
	 */
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb

	/* ---------------------------------------------
	 * Flush all caches to PoC.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	b	dcsw_op_all
endfunc max_generic_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for QEMU "max". Must follow AAPCS.
 */
func max_generic_errata_report
	ret
endfunc max_generic_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cpu specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.max_generic_regs, "aS"
max_generic_regs:  /* The ascii list of register names to be reported */
	.asciz	"" /* no registers to report */

func max_generic_cpu_reg_dump
	adr	x6, max_generic_regs
	ret
endfunc max_generic_cpu_reg_dump


/* cpu_ops for QEMU MAX */
declare_cpu_ops max_generic, QEMU_MAX_MIDR, CPU_NO_RESET_FUNC, \
	max_generic_core_pwr_dwn, \
	max_generic_cluster_pwr_dwn
