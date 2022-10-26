/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>

.global	el3_init_unchanged_regs
.global	el3_init_root_regs

/* --------------------------------------------------------------------------
 * void el3_init_unchanged_regs(void)
 *
 * Initialize EL3 registers which do not change during TF-A lifecycle as
 * they are not part of context management routines
 *
 * --------------------------------------------------------------------------
 */

func el3_init_unchanged_regs

	/* ---------------------------------------------------------------------
	 * SCTLR has already been initialised - read current value before
	 * modifying.
	 *
	 * SCTLR.I: Enable the instruction cache.
	 *
	 * SCTLR.A: Enable Alignment fault checking. All instructions that load
	 *  or store one or more registers have an alignment check that the
	 *  address being accessed is aligned to the size of the data element(s)
	 *  being accessed.
	 * ---------------------------------------------------------------------
	 */

	ldr	r1, =(SCTLR_I_BIT | SCTLR_A_BIT)
	ldcopr	r0, SCTLR
	orr	r0, r0, r1
	stcopr	r0, SCTLR
	isb

	/* ---------------------------------------------------------------------
	 * Initialise NSACR, setting all the fields, except for the
	 * IMPLEMENTATION DEFINED field, rather than relying on the hw. Some
	 * fields are architecturally UNKNOWN on reset.
	 *
	 * NSACR_ENABLE_FP_ACCESS: Represents NSACR.cp11 and NSACR.cp10. The
	 *  cp11 field is ignored, but is set to same value as cp10. The cp10
	 *  field is set to allow access to Advanced SIMD and floating point
	 *  features from both Security states.
	 *
	 * NSACR.NSTRCDIS: When system register trace implemented, Set to one
	 *  so that NS System register accesses to all implemented trace
	 *  registers are disabled.
	 *  When system register trace is not implemented, this bit is RES0 and
	 *  hence set to zero.
	 * ---------------------------------------------------------------------
	 */
	ldcopr	r0, NSACR
	and	r0, r0, #NSACR_IMP_DEF_MASK
	orr	r0, r0, #(NSACR_RESET_VAL | NSACR_ENABLE_FP_ACCESS)
	ldcopr	r1, ID_DFR0
	ubfx	r1, r1, #ID_DFR0_COPTRC_SHIFT, #ID_DFR0_COPTRC_LENGTH
	cmp	r1, #ID_DFR0_COPTRC_SUPPORTED
	bne	1f
	orr	r0, r0, #NSTRCDIS_BIT
1:
	stcopr	r0, NSACR
	isb

	/* ---------------------------------------------------------------------
	 * Initialise CPACR, setting all fields rather than relying on hw. Some
	 * fields are architecturally UNKNOWN on reset.
	 *
	 * CPACR.TRCDIS: Trap control for PL0 and PL1 System register accesses
	 *  to trace registers. Set to zero to allow access.
	 *
	 * CPACR_ENABLE_FP_ACCESS: Represents CPACR.cp11 and CPACR.cp10. The
	 *  cp11 field is ignored, but is set to same value as cp10. The cp10
	 *  field is set to allow full access from PL0 and PL1 to floating-point
	 *  and Advanced SIMD features.
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =((CPACR_RESET_VAL | CPACR_ENABLE_FP_ACCESS) & ~(TRCDIS_BIT))
	stcopr	r0, CPACR
	isb

	/* ---------------------------------------------------------------------
	 * Initialise FPEXC, setting all fields rather than relying on hw. Some
	 * fields are architecturally UNKNOWN on reset and are set to zero
	 * except for field(s) listed below.
	 *
	 * FPEXC.EN: Enable access to Advanced SIMD and floating point features
	 *  from all exception levels.
         *
         * __SOFTFP__: Predefined macro exposed by soft-float toolchain.
         *  ARMv7 and Cortex-A32(ARMv8/aarch32) has both soft-float and
         *  hard-float variants of toolchain, avoid compiling below code with
         *  soft-float toolchain as "vmsr" instruction will not be recognized.
	 * ---------------------------------------------------------------------
	 */
#if ((ARM_ARCH_MAJOR > 7) || defined(ARMV7_SUPPORTS_VFP)) && !(__SOFTFP__)
	ldr	r0, =(FPEXC_RESET_VAL | FPEXC_EN_BIT)
	vmsr	FPEXC, r0
	isb
#endif

#if (ARM_ARCH_MAJOR > 7)
	/* ---------------------------------------------------------------------
	 * Initialise SDCR, setting all the fields rather than relying on hw.
	 *
	 * SDCR.SPD: Disable AArch32 privileged debug. Debug exceptions from
	 *  Secure EL1 are disabled.
	 *
	 * SDCR.SCCD: Set to one so that cycle counting by PMCCNTR is prohibited
	 *  in Secure state. This bit is RES0 in versions of the architecture
	 *  earlier than ARMv8.5, setting it to 1 doesn't have any effect on
	 *  them.
	 *
	 * SDCR.TTRF: Set to one so that access to trace filter control
	 *  registers in non-monitor mode generate Monitor trap exception,
	 *  unless the access generates a higher priority exception when
	 *  trace filter control(FEAT_TRF) is implemented.
	 *  When FEAT_TRF is not implemented, this bit is RES0.
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =((SDCR_RESET_VAL | SDCR_SPD(SDCR_SPD_DISABLE) | \
		      SDCR_SCCD_BIT) & ~SDCR_TTRF_BIT)
	ldcopr	r1, ID_DFR0
	ubfx	r1, r1, #ID_DFR0_TRACEFILT_SHIFT, #ID_DFR0_TRACEFILT_LENGTH
	cmp	r1, #ID_DFR0_TRACEFILT_SUPPORTED
	bne	1f
	orr	r0, r0, #SDCR_TTRF_BIT
1:
	stcopr	r0, SDCR

	/* -----------------------------------------------------
	 * Enable the Asynchronous data abort now that the
	 * exception vectors have been setup.
	 * -----------------------------------------------------
	 */
	cpsie   a
	isb

#endif
	bx	lr
endfunc el3_init_unchanged_regs

/* --------------------------------------------------------------------------
 * void el3_init_unchanged_regs(void)
 *
 * Initialise EL3 registers for its own execution (root context) and these
 * registers will be modified as part of context management during world
 * switches
 * --------------------------------------------------------------------------
 */

func el3_init_root_regs

	/* ---------------------------------------------------------------------
	 * Initialise PMCR, setting all fields rather than relying
	 * on hw. Some fields are architecturally UNKNOWN on reset.
	 *
	 * PMCR.LP: Set to one so that event counter overflow, that
	 *  is recorded in PMOVSCLR[0-30], occurs on the increment
	 *  that changes PMEVCNTR<n>[63] from 1 to 0, when ARMv8.5-PMU
	 *  is implemented. This bit is RES0 in versions of the architecture
	 *  earlier than ARMv8.5, setting it to 1 doesn't have any effect
	 *  on them.
	 *  This bit is Reserved, UNK/SBZP in ARMv7.
	 *
	 * PMCR.LC: Set to one so that cycle counter overflow, that
	 *  is recorded in PMOVSCLR[31], occurs on the increment
	 *  that changes PMCCNTR[63] from 1 to 0.
	 *  This bit is Reserved, UNK/SBZP in ARMv7.
	 *
	 * PMCR.DP: Set to one to prohibit cycle counting whilst in Secure mode.
	 * ---------------------------------------------------------------------
	 */

#if (ARM_ARCH_MAJOR > 7)
	ldr	r0, =(PMCR_RESET_VAL | PMCR_DP_BIT | PMCR_LC_BIT | \
		      PMCR_LP_BIT)
#else
	ldr	r0, =(PMCR_RESET_VAL | PMCR_DP_BIT)
#endif
	stcopr	r0, PMCR

	/* ---------------------------------------------------------------------
	 * Initialise SCR, setting all fields rather than relying on the hw.
	 *
	 * SCR.SIF: Enabled so that Secure state instruction fetches from
	 *  Non-secure memory are not permitted.
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =(SCR_RESET_VAL | SCR_SIF_BIT)
	stcopr	r0, SCR

	/*
	 * If Data Independent Timing (DIT) functionality is implemented,
	 * always enable DIT in EL3
	 */
	ldcopr	r0, ID_PFR0
	and	r0, r0, #(ID_PFR0_DIT_MASK << ID_PFR0_DIT_SHIFT)
	cmp	r0, #ID_PFR0_DIT_SUPPORTED
	bne	1f
	mrs	r0, cpsr
	orr	r0, r0, #CPSR_DIT_BIT
	msr	cpsr_cxsf, r0
1:
	bx	lr
endfunc el3_init_root_regs
