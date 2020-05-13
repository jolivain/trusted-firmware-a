/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <common/bl_common.h>
#include <platform_def.h>
#include "../fpga_private.h"

	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	platform_mem_init
	.globl	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush

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
	dmb	ish

	adrp	x0, topology_tree_spinlock
	add	x0, x0, :lo12:topology_tree_spinlock
	bl	spin_lock

	/*
	 * Create an ID by "glueing" all the affinity levels together
	 * discarding the bits that are not needed on each affinity level.
	 */
	mov	x0, xzr
	mrs	x9, mpidr_el1

	/* Significant bits for afflvl0	*/
	bfi	w0, w9, #0, #FPGA_MAX_PE_PER_CPU_SHIFT
	/* Significant bits for afflvl1	*/
	ubfx	w3, w9, #MPIDR_AFF1_SHIFT, #FPGA_MAX_CPUS_PER_CLUSTER_SHIFT
	bfi	w0, w3, #CORE_MAP_AFF1_SHIFT, #FPGA_MAX_CPUS_PER_CLUSTER_SHIFT
	/* Significant bits for afflvl2 */
	ubfx	w3, w9, #MPIDR_AFF2_SHIFT, #FPGA_MAX_CLUSTER_COUNT_SHIFT
	bfi	w0, w3, #CORE_MAP_AFF2_SHIFT, #FPGA_MAX_CLUSTER_COUNT_SHIFT

	/*
	 * Use the index to update a bitmap with all the cores that showed up.
	 */
	and	x3, x0, #7
	mov	x4, #1
	lsr	x3, x4, x3
	lsr	x0, x0, #3

	adrp	x4, fpga_core_bitmap
	add	x4, x4, :lo12:fpga_core_bitmap
	ldrb	w5, [x4, x0]
	orr	x5, x5, x3
	strb	w5, [x4, x0]

	/* Store the MT bit for this core on an MT bitmap*/
	mov	x0, xzr
	bfi	w0, w9, #0, #FPGA_MAX_CPUS_PER_CLUSTER_SHIFT
	ubfx	w3, w9, #MPIDR_AFF2_SHIFT, #FPGA_MAX_CLUSTER_COUNT_SHIFT
	bfi	w0, w3, #FPGA_MAX_CPUS_PER_CLUSTER_SHIFT, \
						#FPGA_MAX_CLUSTER_COUNT_SHIFT

	and	x3, x0, #7
	mov	x4, #1
	lsr	x3, x4, x3
	lsr	x0, x0, #3

	adrp	x4, mt_bitmap
	add	x4, x4, :lo12:mt_bitmap
	ldrb	w5, [x4, x0]
	orr	x5, x5, x3
	strb	w5, [x4, x0]

	adrp	x0, topology_tree_spinlock
	add	x0, x0, :lo12:topology_tree_spinlock
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

	/*
	 * Before exiting, initialise the per-cpu cache pointer to the CPU
	 * as the early initialisation was performed with a "provisional"
	 * CPUID, whereas now it is possible to calculate the right one.
	 */

	bl	init_cpu_data_ptr

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
	/*
	 * In order to get the right core position it is necessary for the
	 * power domain topology tree to be initialised so the system
	 * topology can be known. This can only happen when all the
	 * cores have been initialised as well.
	 * This causes a circular dependency, as this function also needs
	 * to be called in order to set-up the cores.
	 *
	 * So to break the circular dependency, check first if the system
	 * is initialising. If it is, calculate a "provisional" core
	 * pos based on the hardcoded limits and if it is not, just
	 * return a core pos based on the discovered topology.
	 *
	 * There are two scenarios in which this function is called before
	 * having the topology discovered:
	 * 1.- As part of the cpu data structures initialization for each CPU.
	 * 2.- As part of the stack initialization for the primary CPU.
	 *
	 * In those cases, fallback and get a provisional CPUID to work with.
	 * For the main CPU this will not be a problem as it can only have
	 * ID 0. For the rest of CPUs, just reinitialise the data structures
	 * on the fist time they are powered on, after the topology has been
	 * discovered.
	 *
	 * NOTE: This function accounts for tpidr_el3 to be cleared as part of
	 * a reset, so it can be used to know if the system is initialised or
	 * or not.
	 */

	mrs	x0, mpidr_el1
	/* Only COREID 0 can be the primary CPU */
	mov_imm	x1, MPIDR_AFFINITY_MASK
	and	x2, x0, x1
	cmp	x2, #FPGA_PRIMARY_CPU
	b.ne	1f
	mov	x0, xzr
	ret
1:
	adrp	x2, fpga_power_domain_tree_desc
	add	x2, x2, :lo12:fpga_power_domain_tree_desc
	mrs	x1, tpidr_el3
	cbnz	x1, 2f

	/*
	 * If we are booting up, clear fpga_power_domain_tree_desc[0] in case
	 * the tree is dirty from a previous run, for instance if the system
	 * if rebooting due to a reset pin assertion, as we cannot ensure that
	 * the memory would be cleared in that case.
	 * This way, we ensure that we return a provisional CPUID if this
	 * function is called more than once for the same CPU before the tree
	 * is ready.
	 */
	strb	wzr, [x2, #0]
	b	plat_fpga_calc_provisional_core_pos
2:
	ldrb	w2, [x2, #0]
	cbnz	x2, plat_core_pos_by_mpidr

	/* Fall back and get a provisional core pos */

endfunc plat_my_core_pos

func plat_fpga_calc_provisional_core_pos
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

	mov	x4, #FPGA_MAX_CPUS_PER_CLUSTER
	mov	x5, #FPGA_MAX_PE_PER_CPU

	/* Compute linear position */
	madd	x1, x2, x4, x1
	madd	x0, x1, x5, x0

	ret
endfunc plat_fpga_calc_provisional_core_pos

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
