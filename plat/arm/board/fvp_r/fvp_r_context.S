/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <context.h>
#include <el_max_common_macros.S>

	.global	el2_exit

/* ------------------------------------------------------------------
 * This routine assumes that the SP_EL2 is pointing to a valid
 * context structure from where the gp regs and other special
 * registers can be retrieved.
 * ------------------------------------------------------------------
 */
func el2_exit
#if ENABLE_ASSERTIONS
	/* el2_exit assumes SP_EL0 on entry */
	mrs	x17, spsel
	cmp	x17, #MODE_SP_EL0
	ASM_ASSERT(eq)
#endif

	/* ----------------------------------------------------------
	 * Save the current SP_EL0 i.e. the EL2 runtime stack which
	 * will be used for handling the next SMC.
	 * Then switch to SP_EL2.
	 * ----------------------------------------------------------
	 */
	mov	x17, sp
	msr	spsel, #MODE_SP_ELX
	str	x17, [sp, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]

	/* ----------------------------------------------------------
	 * Restore SPSR_EL3, ELR_EL3 and SCR_EL3 prior to ERET
	 * ----------------------------------------------------------
	 */
	ldr	x18, [sp, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]
	ldp	x16, x17, [sp, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	msr	scr_el3, x18
	msr	spsr_el2, x16
	msr	elr_el2, x17

#if IMAGE_BL31 && DYNAMIC_WORKAROUND_CVE_2018_3639
	/* ----------------------------------------------------------
	 * Restore mitigation state as it was on entry to EL2
	 * ----------------------------------------------------------
	 */
	ldr	x17, [sp, #CTX_CVE_2018_3639_OFFSET + CTX_CVE_2018_3639_DISABLE]
	cbz	x17, 1f
	blr	x17
1:
#endif
	restore_ptw_el1_sys_regs

	/* ----------------------------------------------------------
	 * Restore general purpose (including x30), PMCR_EL0 and
	 * ARMv8.3-PAuth registers.
	 * Exit EL2 via ERET to a lower exception level.
 	 * ----------------------------------------------------------
 	 */
	bl	restore_gp_pmcr_pauth_regs
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

#if IMAGE_BL31 && RAS_EXTENSION
	/* ----------------------------------------------------------
	 * Issue Error Synchronization Barrier to synchronize SErrors
	 * before exiting EL2. We're running with EAs unmasked, so
	 * any synchronized errors would be taken immediately;
	 * therefore no need to inspect DISR_EL1 register.
 	 * ----------------------------------------------------------
	 */
	esb
#endif
	exception_return

endfunc el2_exit
