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
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_fpga_calc_provisional_core_pos
	.globl	get_cpuid_by_mpidr

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

	/*
	 * Use a call to plat_fpga_calc_provisional_core_pos to get the index
	 * where to store this core's MPID.
	 */
	mov	x10, x30
	mrs	x0, mpidr_el1
	bl	plat_fpga_calc_provisional_core_pos
	mov	x30, x10
	mrs	x9, mpidr_el1

	adrp	x4, fpga_mpids
	add	x4, x4, :lo12:fpga_mpids
	/* Use only the first three levels of affinity */
	ldr	x5, =(MPID_MASK & ~(MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT))
	and	x9, x9, x5
	ldr	x5, =DISCOVERED_MPID_MASK
	orr	x5, x9, x5
	str	w5, [x4, x0, LSL #2]

	/*
	 * Wait until the power topology tree is ready to move on as it will
	 * be needed to get the right CPU hold entry.
	 */
	adrp	x4, fpga_power_domain_tree_desc
	add	x4, x4, :lo12:fpga_power_domain_tree_desc
1:
	ldrb	w5, [x4]
	cbz	x5, 1b

	/*
	 * Poll the CPU's hold entry until it indicates to jump
	 * to the entrypoint address.
	 */
	mrs	x0, mpidr_el1
	mov	x1, x30
	bl	get_cpuid_by_mpidr
	mov	x30, x1

	ldr	x1, =-1
	cmp	x0, x1
	b.eq	el3_panic			/* No valid CPUID found */

	adrp	x1, hold_base
	add	x1, x1, :lo12:hold_base
poll_hold_entry:
	ldr	x3, [x1, x0, LSL #PLAT_FPGA_HOLD_ENTRY_SHIFT]
	cmp	x3, #PLAT_FPGA_HOLD_STATE_GO
	b.ne	1f

	adrp	x2, fpga_sec_entrypoint
	add	x2, x2, :lo12:fpga_sec_entrypoint
	ldr	x3, [x2]
	br	x3
1:
	wfe
	b	poll_hold_entry

endfunc plat_secondary_cold_boot_setup

/* -----------------------------------------------------------------------
 * int get_cpuid_by_mpidr(u_register_t mpidr);
 *
 * Return the CPUID registered for a given mpid value or -1 if not found.
 * This function does not need a stack frame.
 *
 * Note: This function must not clobber x10, as it will be called
 *       by init_cpu_data_ptr(), which expects x10 not to be clobbered.
 * -----------------------------------------------------------------------
 */
func get_cpuid_by_mpidr
	ldr	x14, =(MPID_MASK & ~(MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT))
	and	x11, x0, x14
	mrs	x14, mpidr_el1
	ldr	x0, =MPIDR_MT_MASK
	and	x14, x14, x0
	orr	x11, x11, x14
	adrp	x14, fpga_mpids
	add	x14, x14, :lo12:fpga_mpids	/* Start addr */
	mov	x15, #PLATFORM_CORE_COUNT
	lsl	x15, x15, #2
	add	x15, x14, x15			/* End addr */
	mov	x0, xzr
mpidr_list_loop:
	ldr	w12, [x14], #4
	and	x13, x12, #DISCOVERED_MPID_MASK
	cbz	x13, 1f				/* MPID not discovered, skip */
	and	x12, x12, #~DISCOVERED_MPID_MASK
	cmp	x11, x12
	b.ne	2f				/* Not found, keep going */
	ret					/* CPUID found */
2:
	add	x0, x0, #1			/* Increment the CPUID count
						   if this was a valid MPID */
1:
	cmp	x14, x15
	b.ne	mpidr_list_loop			/* Keep on iterating */
	ldr	x0, =-1				/* MPID not valid */
	ret

endfunc get_cpuid_by_mpidr

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
	 * In order to get the right core pos it is necessary for the power
	 * domain topology tree to be initialised. There are, however,
	 * some situations in which this function will be called before
	 * the tree is ready, causing a circular dependency:
	 *
	 * 1.- As part of the cpu data structures initialisation for each CPU.
	 * 2.- As part of the stack initialization for the primary CPU.
	 *
	 * For the above cases, the circular dependency can be broken by
	 * getting a provisional cpu pos as:
	 *
	 * A) The secondary CPUs can reinitialise its cpu data structures
	 *    after the topology has been discovered, before jumping to their
	 *    entrypoint.
	 * B) when RESET_TO_BL31 is enabled, the primary CPU can only have
	 * id 0, so just return 0 when called from the primary CPU.
	 *
	 * NOTE: This function accounts for tpidr_el3 to be cleared as part of
	 * a reset, so its value can be used to know if the system is
	 * initialised or not.
	 */

	mov	x11, x30
	bl	plat_is_my_cpu_primary
	mov	x30, x11
	cbz	x0, 1f
	/* If this is the primary CPU, its ID is 0 */
	mov	x0, xzr
	ret
1:
	adrp	x2, fpga_power_domain_tree_desc
	add	x2, x2, :lo12:fpga_power_domain_tree_desc
	mrs	x0, mpidr_el1
	mrs	x1, tpidr_el3
	cbnz	x1, 2f

	/*
	 * If we are booting up, clear fpga_power_domain_tree_desc[0] in case
	 * the tree is dirty from a previous run, for instance if the system
	 * is rebooting due to a reset pin assertion, as we cannot ensure that
	 * the memory would be cleared in that case.
	 * This way, we ensure that we return a provisional CPUID if this
	 * function is called more than once for the same CPU before the tree
	 * is ready.
	 */
	strb	wzr, [x2]
	b	plat_fpga_calc_provisional_core_pos
2:
	ldrb	w2, [x2]
	cbnz	x2, get_cpuid_by_mpidr

	/* Carry on by getting a provisional core pos */

endfunc plat_my_core_pos

/* -----------------------------------------------------------------------
 * unsigned int plat_fpga_calc_provisional_core_pos (uint32_t mpid)
 *
 * This function returns a CPUID based on the harcoded limits for the
 * topology rather than the discovered ones, so it can be used to get
 * provisional CPUIDs before the power topology tree is fully discovered.
 * -----------------------------------------------------------------------
 */
func plat_fpga_calc_provisional_core_pos
	/*
	 * Check for MT bit in MPIDR, which may be either value for images
	 * running on the FPGA.
	 *
	 * If not set, shift MPIDR to left to make it look as if in a
	 * multi-threaded implementation.
	 *
	 * Clobber registers: r0 to r5
	 */
	tst	x0, #MPIDR_MT_MASK
	lsl	x3, x0, #MPIDR_AFFINITY_BITS
	csel	x3, x3, x0, eq

	/* Extract individual affinity fields from MPIDR */
	ubfx	x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	cmp	x0, #FPGA_MAX_PE_PER_CPU
	b.ge    el3_panic
	ubfx	x1, x3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	cmp	x1, #FPGA_MAX_CPUS_PER_CLUSTER
	b.ge	el3_panic
	ubfx	x2, x3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS
	cmp	x2, #FPGA_MAX_CLUSTER_COUNT
	b.ge	el3_panic

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
