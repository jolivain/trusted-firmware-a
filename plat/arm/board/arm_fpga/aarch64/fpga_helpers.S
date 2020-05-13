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
	.globl	plat_fpga_calc_core_pos
	.globl	fpga_get_cpu_affinity
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	fpga_update_affinity_boundaries

/* ------------------------------------------------------------------------
 * Ancillary function to keep track of the highest possible value for CPUID
 * and ThreadID on an MPIDR. This is needed in order to allow
 * plat_core_pos_by_mpidr() to reject any unallocated MPIDR.
 * (see section 6.2.2 on
 * https://trustedfirmware-a.readthedocs.io/en/latest/design/psci-pd-tree.html)
 *
 * By registering the boundaries for the affinity levels, it is not
 * necessary to walk the entire topology tree every time
 * plat_core_pos_by_mpidr() is called, thus providing better performance.
 * As a drawback, this implemention assumes that the topology is consistent
 * accross clusters and CPUs and will not work properly otherwise.
 *
 * The affinity boundaries for the CPUs and Threads are stored on
 * fpga_max_cpu_affvl and fpga_max_th_affvl respectively.
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
	/* Update the max thread affvl */
	adrp	x4, fpga_max_th_affvl
	add	x4, x4, :lo12:fpga_max_th_affvl
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

	adrp	x0, topology_tree_spinlock
	add	x0, x0, :lo12:topology_tree_spinlock
	bl	spin_lock

	/*
	 * Populate the topology tree with the information of this core.
	 */
	mov	x0, #0
	bl	fpga_get_cpu_affinity
	cmp	x0, #FPGA_MAX_PE_PER_CPU
	b.gt	el3_panic /* Panic if the number of threads is exceeded */

	mov	x0, #1
	bl	fpga_get_cpu_affinity
	cmp	x0, #FPGA_MAX_CPUS_PER_CLUSTER
	b.ge	el3_panic /* Panic if the number of CPUs is exceeded */
	mov	x6, x0

	mov	x0, #2
	bl	fpga_get_cpu_affinity
	cmp	x0, #FPGA_MAX_CLUSTER_COUNT
	b.ge	el3_panic /* Panic if the number of clusters is exceeded */
	mov	x7, x0

	/*
	 * Use the CPU cluster as index for the power domain tree.
	 * First element is the system level, second one, the clusters
	 * and from the third on, the cores in clusters and threads in cores.
	 */
	adrp	x4, fpga_power_domain_tree_desc
	add	x4, x4, :lo12:fpga_power_domain_tree_desc
	mov	x0, #2
	add	x3, x7, x0
	ldrb	w5, [x4, x3]
	add	x5, x5, #1 /* Add this core to its cluster */
	cmp	x5, #FPGA_MAX_CPUS_PER_CLUSTER
	b.hi 	el3_panic /* Panic if the max CPUs per cluster is exceeded */
	strb	w5, [x4, x3]

	/*
	 * It is necessary to update the number of threads for the current CPU,
	 * but since the total number of clusters is still unknown, the count
	 * must be kept in a separate array.
	 */
	adrp	x4, fpga_cpu_topology_tree
	add	x4, x4, :lo12:fpga_cpu_topology_tree
	mov	x5, #FPGA_MAX_CPUS_PER_CLUSTER
	madd	x3, x7, x5, x6
	ldrb	w5, [x4, x3]
	add	x5, x5, #1 /* Add this tread to its CPU */
	/*
	 * TODO: The assumption here is that every CPU has at least one thread
	 * even when the CPU is not MT. This is done in order to keep
	 * consistency with plat_fpga_calc_core_pos(), which shifts the MPIDR
	 * if the CPU is not multithread to look it like multithread with a
	 * single Thread ID of 0.
	 * It is necessary to revisit this approach, though.
	 */
	strb	w5, [x4, x3]

	bl	fpga_update_affinity_boundaries

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
 * unsigned int fpga_get_cpu_affinity(unsigned int afflvl)
 * -----------------------------------------------------------------------
 */
func fpga_get_cpu_affinity
	mrs	x1, mpidr_el1

	/*
	 * Check for MT bit in MPIDR, which may be either value for images
	 * running on the FPGA.
	 *
	 * If not set, shift MPIDR to left to make it look as if in a
	 * multi-threaded implementation.
	 */
	tst	x1, #MPIDR_MT_MASK
	lsl	x3, x1, #MPIDR_AFFINITY_BITS
	csel	x3, x3, x1, eq

	/* Extract individual affinity fields from MPIDR */
	cmp	x0, #0
	b.ne	1f
	ubfx	x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ret
1:
	cmp	x0, #1
	b.eq	2f
	ubfx	x0, x3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS
	ret
2:
	ubfx	x0, x3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ret
endfunc fpga_get_cpu_affinity

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
