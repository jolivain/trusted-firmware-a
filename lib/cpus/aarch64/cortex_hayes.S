/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_hayes.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex Hayes must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex Hayes supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_hayes_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_HAYES_CPUPWRCTLR_EL1, CORTEX_HAYES_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_hayes_core_pwr_dwn

errata_report_shim cortex_hayes

cpu_reset_func_start cortex_hayes
cpu_reset_func_end cortex_hayes

	/* ---------------------------------------------
	 * This function provides Cortex Hayes specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_hayes_regs, "aS"
cortex_hayes_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_hayes_cpu_reg_dump
	adr	x6, cortex_hayes_regs
	mrs	x8, CORTEX_HAYES_CPUECTLR_EL1
	ret
endfunc cortex_hayes_cpu_reg_dump

declare_cpu_ops cortex_hayes, CORTEX_HAYES_MIDR, \
	cortex_hayes_reset_func, \
	cortex_hayes_core_pwr_dwn
