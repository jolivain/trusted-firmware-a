/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <platform_def.h>
#include "../fpga_private.h"

	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	platform_mem_init
	.globl	plat_my_core_pos
	.globl	plat_fpga_calc_core_pos
	.globl	fpga_get_cpu_cluster
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	fpga_update_affinity_boundaries

.section .data
	define_asm_spinlock topology_tree_spinlock

/* ------------------------------------------------------------------------
 * Update the number of CPUs per cluster as well as threads per CPUs as the
 * current topology tree implementation does not make distinctions between
 * them. This is needed for instance on plat_core_pos_by_mpidr()
 * to avoid returning unexisting core IDs.
 *
 * This function asumes that all the clusters and CPUs have the same
 * number of CPUs and PEs, respectively.
 * ------------------------------------------------------------------------
 */
func fpga_update_affinity_boundaries
	mrs	x0, mpidr_el1

	/*
	 * Check for MT bit in MPIDR, which may be either value for images
	 * running on the FPGA.
	 *
	 * If not set, shift MPIDR to left to make it look as if in a
	 * multi-threaded implementation.
	 */
	tst	x0, #MPIDR_MT_MASK
	lsl	x3, x0, #MPIDR_AFFINITY_BITS
	csel	x3, x3, x0, eq

	/* Extract individual affinity fields from MPIDR */
	ubfx	x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x1, x3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS

	/* Update the max cpu affvl */
	adrp	x4, fpga_max_cpu_affvl
	add	x4, x4, :lo12:fpga_max_cpu_affvl
	ldrb	w5, [x4]
	/*
	 * Compare the afflvl of the current CPU with the maximum value
	 * obtained until now.
	 */
	cmp	x5, x0
	b.ge	1f
	add	x5, x5, #1
	strb	w5, [x4]
1:
	/* Update the max pe affvl */
	adrp	x4, fpga_max_pe_affvl
	add	x4, x4, :lo12:fpga_max_pe_affvl
	ldrb	w5, [x4]
	/*
	 * Compare the afflvl of the current PE with the maximum value
	 * obtained until now.
	 */
	cmp	x5, x1
	b.ge	2f
	add	x5, x5, #1
	strb	w5, [x4]
2:
	ret
endfunc fpga_update_affinity_boundaries

/* -----------------------------------------------------------------------
 * Indicate a cold boot for every CPU - warm boot is unsupported for the
 * holding pen PSCI implementation.
 * -----------------------------------------------------------------------
 */
func plat_get_my_entrypoint
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint

/* -----------------------------------------------------------------------
 * void plat_secondary_cold_boot_setup (void);
 * -----------------------------------------------------------------------
 */
func plat_secondary_cold_boot_setup

	/*
	 * Secondary processors cannot populate the topology tree
	 * until the primary one has finished seting up the C runtime or
	 * otherwise the topology tree would be erased. It is therefore
	 * necessary to keep them waiting in a spin-lock until the C runtime
	 * is ready. We cannot use the current spin-lock implementation until
	 * runtime is up and we should not rely on sevl/wfe instructions as
	 * it is optional whether they are implemented or not, so we use
	 * a global variable as lock and wait for the primary processor to
	 * finish the C runtime bring-up.
	 */

	ldr	w0, =C_RUNTIME_READY_KEY
	adrp	x1, secondary_core_spinlock
	add	x1, x1, :lo12:secondary_core_spinlock
1:
	wfe
	ldr	w2, [x1]
	cmp	w2, w0
	b.ne	1b
	dmb	sy

	/*
	 * Populate the topology tree with the information of this core.
	 */
	bl	fpga_get_cpu_cluster
	cmp	x0, #FPGA_MAX_CLUSTER_COUNT
	b.ge	el3_panic /* Panic if the number of clusters is exceeded */

	/*
	 * Use the CPU cluster as index for the power domain tree.
	 * First element is the system level, second one, the clusters
	 * and from the third on, the cores in clusters.
	 */
	add	x3, x0, #2
	adrp	x4, fpga_power_domain_tree_desc
	add	x4, x4, :lo12:fpga_power_domain_tree_desc
	adrp	x0, topology_tree_spinlock
	add	x0, x0, :lo12:topology_tree_spinlock
	bl	spin_lock
	ldrb	w5, [x4, x3]
	add	x5, x5, #1 /* Add this core to its cluster */
	cmp	x5, #FPGA_MAX_CPUS_PER_CLUSTER
	b.hi 	el3_panic /* Panic if the max CPUs per cluster is exceeded */
	strb	w5, [x4, x3]
	mov	x19, x0
	bl	fpga_update_affinity_boundaries
	mov	x0, x19
	bl	spin_unlock

	/*
	 * Poll the CPU's hold entry until it indicates to jump
	 * to the entrypoint address.
	 */
	bl	plat_my_core_pos
	lsl	x0, x0, #PLAT_FPGA_HOLD_ENTRY_SHIFT
	ldr	x1, =hold_base
	ldr	x2, =fpga_sec_entrypoint
poll_hold_entry:
	ldr	x3, [x1, x0]
	cmp	x3, #PLAT_FPGA_HOLD_STATE_GO
	b.ne	1f
	ldr	x3, [x2]
	br	x3
1:
	wfe
	b	poll_hold_entry
endfunc plat_secondary_cold_boot_setup

/* -----------------------------------------------------------------------
 * unsigned int plat_is_my_cpu_primary (void);
 *
 * Find out whether the current cpu is the primary cpu
 * -----------------------------------------------------------------------
 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	mov_imm	x1, MPIDR_AFFINITY_MASK
	and	x0, x0, x1
	cmp	x0, #FPGA_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary

func platform_mem_init
	ret
endfunc platform_mem_init

func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_fpga_calc_core_pos
endfunc plat_my_core_pos

/* -----------------------------------------------------------------------
 * unsigned int fpga_get_cpu_cluster(void)
 * -----------------------------------------------------------------------
 */
func fpga_get_cpu_cluster
	mrs	x0, mpidr_el1

	/*
	 * Check for MT bit in MPIDR, which may be either value for images
	 * running on the FPGA.
	 *
	 * If not set, shift MPIDR to left to make it look as if in a
	 * multi-threaded implementation.
	 */
	tst	x0, #MPIDR_MT_MASK
	lsl	x3, x0, #MPIDR_AFFINITY_BITS
	csel	x3, x3, x0, eq

	/* Extract individual affinity fields from MPIDR */
	ubfx	x0, x3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS

	ret
endfunc fpga_get_cpu_cluster

/* -----------------------------------------------------------------------
 * unsigned int plat_fpga_calc_core_pos(u_register_t mpidr)
 * -----------------------------------------------------------------------
 */
func plat_fpga_calc_core_pos
	/*
	 * Check for MT bit in MPIDR, which may be either value for images
	 * running on the FPGA.
	 *
	 * If not set, shift MPIDR to left to make it look as if in a
	 * multi-threaded implementation.
	 */
	tst	x0, #MPIDR_MT_MASK
	lsl	x3, x0, #MPIDR_AFFINITY_BITS
	csel	x3, x3, x0, eq

	/* Extract individual affinity fields from MPIDR */
	ubfx	x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x1, x3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x2, x3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS

	/* Compute linear position */
	mov	x4, #FPGA_MAX_CPUS_PER_CLUSTER
	madd	x1, x2, x4, x1
	mov	x5, #FPGA_MAX_PE_PER_CPU
	madd	x0, x1, x5, x0
	ret
endfunc plat_fpga_calc_core_pos

func plat_crash_console_init
	mov_imm	x0, PLAT_FPGA_CRASH_UART_BASE
	b	console_pl011_core_init
endfunc plat_crash_console_init

func plat_crash_console_putc
	mov_imm	x1, PLAT_FPGA_CRASH_UART_BASE
	b	console_pl011_core_putc
endfunc plat_crash_console_putc

func plat_crash_console_flush
	mov_imm	x0, PLAT_FPGA_CRASH_UART_BASE
	b	console_pl011_core_flush
endfunc plat_crash_console_flush
