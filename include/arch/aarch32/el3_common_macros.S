/*
 * Copyright (c) 2016-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EL3_COMMON_MACROS_S
#define EL3_COMMON_MACROS_S

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <lib/xlat_tables/xlat_tables_defs.h>

#define PAGE_START_MASK		~(PAGE_SIZE_MASK)

	/*
	 * Helper macro to initialise EL3 registers we care about.
	 */
	.macro el3_arch_init_common

	/* -------------------------------------------------------------------
	 * Initialize EL3 registers which do not change during TF-A lifecycle.
	 * These registers are not part of any context save/restore routines.
	 * Following registers modified in this routine
	 *	- SCTLR
	 *	- NSACR
	 *	- CPACR
	 *	- SDCR
	 *	- FPEXC
	 * -------------------------------------------------------------------
	 */

	bl	el3_init_unchanged_regs

	/* -----------------------------------------------------------------------
	 * Initialise EL3 registers for its own execution (root context). These
	 * register values will be saved when exiting from EL3 and new values will
	 * be set based on the world it transition into.
	 * Following registers modified in this routine
	 *	- SCR
	 *	- PMCR
	 *	- CPSR
	 * -----------------------------------------------------------------------
	 */

	bl	el3_init_root_regs

	.endm

/* -----------------------------------------------------------------------------
 * This is the super set of actions that need to be performed during a cold boot
 * or a warm boot in EL3. This code is shared by BL1 and BL32 (SP_MIN).
 *
 * This macro will always perform reset handling, architectural initialisations
 * and stack setup. The rest of the actions are optional because they might not
 * be needed, depending on the context in which this macro is called. This is
 * why this macro is parameterised ; each parameter allows to enable/disable
 * some actions.
 *
 *  _init_sctlr:
 *	Whether the macro needs to initialise the SCTLR register including
 *	configuring the endianness of data accesses.
 *
 *  _warm_boot_mailbox:
 *	Whether the macro needs to detect the type of boot (cold/warm). The
 *	detection is based on the platform entrypoint address : if it is zero
 *	then it is a cold boot, otherwise it is a warm boot. In the latter case,
 *	this macro jumps on the platform entrypoint address.
 *
 *  _secondary_cold_boot:
 *	Whether the macro needs to identify the CPU that is calling it: primary
 *	CPU or secondary CPU. The primary CPU will be allowed to carry on with
 *	the platform initialisations, while the secondaries will be put in a
 *	platform-specific state in the meantime.
 *
 *	If the caller knows this macro will only be called by the primary CPU
 *	then this parameter can be defined to 0 to skip this step.
 *
 * _init_memory:
 *	Whether the macro needs to initialise the memory.
 *
 * _init_c_runtime:
 *	Whether the macro needs to initialise the C runtime environment.
 *
 * _exception_vectors:
 *	Address of the exception vectors to program in the VBAR_EL3 register.
 *
 * _pie_fixup_size:
 *	Size of memory region to fixup Global Descriptor Table (GDT).
 *
 *	A non-zero value is expected when firmware needs GDT to be fixed-up.
 *
 * -----------------------------------------------------------------------------
 */
	.macro el3_entrypoint_common					\
		_init_sctlr, _warm_boot_mailbox, _secondary_cold_boot,	\
		_init_memory, _init_c_runtime, _exception_vectors,	\
		_pie_fixup_size

	/* Make sure we are in Secure Mode */
#if ENABLE_ASSERTIONS
	ldcopr	r0, SCR
	tst	r0, #SCR_NS_BIT
	ASM_ASSERT(eq)
#endif

	.if \_init_sctlr
		/* -------------------------------------------------------------
		 * This is the initialisation of SCTLR and so must ensure that
		 * all fields are explicitly set rather than relying on hw. Some
		 * fields reset to an IMPLEMENTATION DEFINED value.
		 *
		 * SCTLR.TE: Set to zero so that exceptions to an Exception
		 *  Level executing at PL1 are taken to A32 state.
		 *
		 * SCTLR.EE: Set the CPU endianness before doing anything that
		 *  might involve memory reads or writes. Set to zero to select
		 *  Little Endian.
		 *
		 * SCTLR.V: Set to zero to select the normal exception vectors
		 *  with base address held in VBAR.
		 *
		 * SCTLR.DSSBS: Set to zero to disable speculation store bypass
		 *  safe behaviour upon exception entry to EL3.
		 * -------------------------------------------------------------
		 */
		ldr     r0, =(SCTLR_RESET_VAL & ~(SCTLR_TE_BIT | SCTLR_EE_BIT | \
				SCTLR_V_BIT | SCTLR_DSSBS_BIT))
		stcopr	r0, SCTLR
		isb
	.endif /* _init_sctlr */

	/* Switch to monitor mode */
	cps	#MODE32_mon
	isb

#if DISABLE_MTPMU
	bl	mtpmu_disable
#endif

	.if \_warm_boot_mailbox
		/* -------------------------------------------------------------
		 * This code will be executed for both warm and cold resets.
		 * Now is the time to distinguish between the two.
		 * Query the platform entrypoint address and if it is not zero
		 * then it means it is a warm boot so jump to this address.
		 * -------------------------------------------------------------
		 */
		bl	plat_get_my_entrypoint
		cmp	r0, #0
		bxne	r0
	.endif /* _warm_boot_mailbox */

	.if \_pie_fixup_size
#if ENABLE_PIE
		/*
		 * ------------------------------------------------------------
		 * If PIE is enabled fixup the Global descriptor Table only
		 * once during primary core cold boot path.
		 *
		 * Compile time base address, required for fixup, is calculated
		 * using "pie_fixup" label present within first page.
		 * ------------------------------------------------------------
		 */
	pie_fixup:
		ldr	r0, =pie_fixup
		ldr	r1, =PAGE_START_MASK
		and	r0, r0, r1
		mov_imm	r1, \_pie_fixup_size
		add	r1, r1, r0
		bl	fixup_gdt_reloc
#endif /* ENABLE_PIE */
	.endif /* _pie_fixup_size */

	/* ---------------------------------------------------------------------
	 * Set the exception vectors (VBAR/MVBAR).
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =\_exception_vectors
	stcopr	r0, VBAR
	stcopr	r0, MVBAR
	isb

	/* ---------------------------------------------------------------------
	 * It is a cold boot.
	 * Perform any processor specific actions upon reset e.g. cache, TLB
	 * invalidations etc.
	 * ---------------------------------------------------------------------
	 */
	bl	reset_handler

	el3_arch_init_common

	.if \_secondary_cold_boot
		/* -------------------------------------------------------------
		 * Check if this is a primary or secondary CPU cold boot.
		 * The primary CPU will set up the platform while the
		 * secondaries are placed in a platform-specific state until the
		 * primary CPU performs the necessary actions to bring them out
		 * of that state and allows entry into the OS.
		 * -------------------------------------------------------------
		 */
		bl	plat_is_my_cpu_primary
		cmp	r0, #0
		bne	do_primary_cold_boot

		/* This is a cold boot on a secondary CPU */
		bl	plat_secondary_cold_boot_setup
		/* plat_secondary_cold_boot_setup() is not supposed to return */
		no_ret	plat_panic_handler

	do_primary_cold_boot:
	.endif /* _secondary_cold_boot */

	/* ---------------------------------------------------------------------
	 * Initialize memory now. Secondary CPU initialization won't get to this
	 * point.
	 * ---------------------------------------------------------------------
	 */

	.if \_init_memory
		bl	platform_mem_init
	.endif /* _init_memory */

	/* ---------------------------------------------------------------------
	 * Init C runtime environment:
	 *   - Zero-initialise the NOBITS sections. There are 2 of them:
	 *       - the .bss section;
	 *       - the coherent memory section (if any).
	 *   - Relocate the data section from ROM to RAM, if required.
	 * ---------------------------------------------------------------------
	 */
	.if \_init_c_runtime
#if defined(IMAGE_BL32) || (defined(IMAGE_BL2) && BL2_AT_EL3)
		/* -----------------------------------------------------------------
		 * Invalidate the RW memory used by the image. This
		 * includes the data and NOBITS sections. This is done to
		 * safeguard against possible corruption of this memory by
		 * dirty cache lines in a system cache as a result of use by
		 * an earlier boot loader stage. If PIE is enabled however,
		 * RO sections including the GOT may be modified during
		 * pie fixup. Therefore, to be on the safe side, invalidate
		 * the entire image region if PIE is enabled.
		 * -----------------------------------------------------------------
		 */
#if ENABLE_PIE
#if SEPARATE_CODE_AND_RODATA
		ldr	r0, =__TEXT_START__
#else
		ldr	r0, =__RO_START__
#endif /* SEPARATE_CODE_AND_RODATA */
#else
		ldr	r0, =__RW_START__
#endif /* ENABLE_PIE */
		ldr	r1, =__RW_END__
		sub	r1, r1, r0
		bl	inv_dcache_range
#if defined(IMAGE_BL2) && SEPARATE_BL2_NOLOAD_REGION
		ldr	r0, =__BL2_NOLOAD_START__
		ldr	r1, =__BL2_NOLOAD_END__
		sub	r1, r1, r0
		bl	inv_dcache_range
#endif
#endif

		/*
		 * zeromem uses r12 whereas it is used to save previous BL arg3,
		 * save it in r7
		 */
		mov	r7, r12
		ldr	r0, =__BSS_START__
		ldr	r1, =__BSS_END__
		sub 	r1, r1, r0
		bl	zeromem

#if USE_COHERENT_MEM
		ldr	r0, =__COHERENT_RAM_START__
		ldr	r1, =__COHERENT_RAM_END_UNALIGNED__
		sub 	r1, r1, r0
		bl	zeromem
#endif

		/* Restore r12 */
		mov	r12, r7

#if defined(IMAGE_BL1) || (defined(IMAGE_BL2) && BL2_AT_EL3 && BL2_IN_XIP_MEM)
		/* -----------------------------------------------------
		 * Copy data from ROM to RAM.
		 * -----------------------------------------------------
		 */
		ldr	r0, =__DATA_RAM_START__
		ldr	r1, =__DATA_ROM_START__
		ldr	r2, =__DATA_RAM_END__
		sub 	r2, r2, r0
		bl	memcpy4
#endif
	.endif /* _init_c_runtime */

	/* ---------------------------------------------------------------------
	 * Allocate a stack whose memory will be marked as Normal-IS-WBWA when
	 * the MMU is enabled. There is no risk of reading stale stack memory
	 * after enabling the MMU as only the primary CPU is running at the
	 * moment.
	 * ---------------------------------------------------------------------
	 */
	bl	plat_set_my_stack

#if STACK_PROTECTOR_ENABLED
	.if \_init_c_runtime
	bl	update_stack_protector_canary
	.endif /* _init_c_runtime */
#endif
	.endm

#endif /* EL3_COMMON_MACROS_S */
