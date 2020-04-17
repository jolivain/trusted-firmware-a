/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>
#include <cpu_macros.S>

	.globl	plat_arm_calc_core_pos
	.globl	plat_reset_handler

	/* -----------------------------------------------------
	 * unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *
	 * Helper function to calculate the core position.
	 * (ChipId * PLAT_ARM_CLUSTER_COUNT *
	 *  PLAT_MAX_CPUS_PER_CLUSTER * PLAT_MAX_PE_PER_CPU) +
	 * (ClusterId * PLAT_MAX_CPUS_PER_CLUSTER * PLAT_MAX_PE_PER_CPU) +
	 * (CPUId * PLAT_MAX_PE_PER_CPU) +
	 * ThreadId
	 *
	 * which can be simplified as:
	 *
	 * ((((ChipId * PLAT_ARM_CLUSTER_COUNT) + ClusterId) *
	 *   PLAT_MAX_CPUS_PER_CLUSTER) + CPUId) * PLAT_MAX_PE_PER_CPU +
	 * ThreadId
	 * ------------------------------------------------------
	 */

func plat_arm_calc_core_pos
	mov	x4, x0

	/*
	 * The MT bit in MPIDR is always set for SGI platforms
	 * and the affinity level 0 corresponds to thread affinity level.
	 */

	/* Extract individual affinity fields from MPIDR */
	ubfx    x0, x4, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx    x1, x4, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx    x2, x4, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx    x3, x4, #MPIDR_AFF3_SHIFT, #MPIDR_AFFINITY_BITS

	/* Compute linear position */
	mov     x4, #PLAT_ARM_CLUSTER_COUNT
	madd    x2, x3, x4, x2
	mov     x4, #PLAT_MAX_CPUS_PER_CLUSTER
	madd    x1, x2, x4, x1
	mov     x4, #PLAT_MAX_PE_PER_CPU
	madd    x0, x1, x4, x0
	ret
endfunc plat_arm_calc_core_pos

	/* -----------------------------------------------------
	 * void plat_reset_handler(void);
	 *
	 * Determine the CPU MIDR and disable power down bit for
	 * that CPU.
	 * -----------------------------------------------------
	 */
func plat_reset_handler
	ret
endfunc plat_reset_handler
