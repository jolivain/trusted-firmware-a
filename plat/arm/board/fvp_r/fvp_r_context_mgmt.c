/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_features.h>
#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>
#include <lib/extensions/mpam.h>
#include <lib/extensions/spe.h>
#include <lib/extensions/sve.h>
#include <lib/extensions/twed.h>
#include <lib/utils.h>
#include <fvp_r_arch_helpers.h>


/*******************************************************************************
 * File contains EL2 equivalents of EL3 functions from:
 * 	.../lib/el3_runtime/aarch64/context_mgmt.c
 ******************************************************************************/

/*******************************************************************************
 * Enable architecture extensions on first entry to Non-secure world.
 * When EL2 is implemented but unused `el2_unused` is non-zero, otherwise
 * it is zero.
 ******************************************************************************/
static void enable_extensions_nonsecure(bool el2_unused)
{
#if IMAGE_BL31
#if ENABLE_SPE_FOR_LOWER_ELS
	spe_enable(el2_unused);
#endif

#if ENABLE_AMU
	amu_enable(el2_unused);
#endif

#if ENABLE_SVE_FOR_NS
	sve_enable(el2_unused);
#endif

#if ENABLE_MPAM_FOR_LOWER_ELS
	mpam_enable(el2_unused);
#endif
#endif
}

/*******************************************************************************
 * Prepare the CPU system registers for first entry into secure or normal world
 *
 * If execution is requested to EL2 or hyp mode, SCTLR_EL2 is initialized
 * If execution is requested to non-secure EL1 or svc mode, and the CPU supports
 * EL2 then EL2 is disabled by configuring all necessary EL2 registers.
 * For all entries, the EL1 registers are initialized from the cpu_context
 ******************************************************************************/
void cm_prepare_el2_exit(uint32_t security_state)
{
	u_register_t sctlr_elx, scr_el3, mdcr_el2;
	cpu_context_t *ctx = cm_get_context(security_state);
	bool el2_unused = false;
	uint64_t hcr_el2 = 0U;

	assert(ctx != NULL);

	if (security_state == NON_SECURE) {
		scr_el3 = read_ctx_reg(get_el3state_ctx(ctx),
						CTX_SCR_EL3);
		if ((scr_el3 & SCR_HCE_BIT) != 0U) {
			/* Use SCTLR_EL1.EE value to initialise sctlr_el2 */
			sctlr_elx = read_ctx_reg(get_el1_sysregs_ctx(ctx),
							CTX_SCTLR_EL1);
			sctlr_elx &= SCTLR_EE_BIT;
			sctlr_elx |= SCTLR_EL2_RES1;
#if ERRATA_A75_764081
			/*
			 * If workaround of errata 764081 for Cortex-A75 is used
			 * then set SCTLR_EL2.IESB to enable Implicit Error
			 * Synchronization Barrier.
			 */
			sctlr_elx |= SCTLR_IESB_BIT;
#endif
			write_sctlr_el2(sctlr_elx);
		} else if (el_implemented(2) != EL_IMPL_NONE) {
			el2_unused = true;

			/*
			 * EL2 present but unused, need to disable safely.
			 * SCTLR_EL2 can be ignored in this case.
			 *
			 * Set EL2 register width appropriately: Set HCR_EL2
			 * field to match SCR_EL3.RW.
			 */
			if ((scr_el3 & SCR_RW_BIT) != 0U)
				hcr_el2 |= HCR_RW_BIT;

			/*
			 * For Armv8.3 pointer authentication feature, disable
			 * traps to EL2 when accessing key registers or using
			 * pointer authentication instructions from lower ELs.
			 */
			hcr_el2 |= (HCR_API_BIT | HCR_APK_BIT);

			write_hcr_el2(hcr_el2);

			/*
			 * Initialise CPTR_EL2 setting all fields rather than
			 * relying on the hw. All fields have architecturally
			 * UNKNOWN reset values.
			 *
			 * CPTR_EL2.TCPAC: Set to zero so that Non-secure EL1
			 *  accesses to the CPACR_EL1 or CPACR from both
			 *  Execution states do not trap to EL2.
			 *
			 * CPTR_EL2.TTA: Set to zero so that Non-secure System
			 *  register accesses to the trace registers from both
			 *  Execution states do not trap to EL2.
			 *
			 * CPTR_EL2.TFP: Set to zero so that Non-secure accesses
			 *  to SIMD and floating-point functionality from both
			 *  Execution states do not trap to EL2.
			 */
			write_cptr_el2(CPTR_EL2_RESET_VAL &
					~(CPTR_EL2_TCPAC_BIT | CPTR_EL2_TTA_BIT
					| CPTR_EL2_TFP_BIT));

			/*
			 * Initialise CNTHCTL_EL2. All fields are
			 * architecturally UNKNOWN on reset and are set to zero
			 * except for field(s) listed below.
			 *
			 * CNTHCTL_EL2.EL1PCEN: Set to one to disable traps to
			 *  Hyp mode of Non-secure EL0 and EL1 accesses to the
			 *  physical timer registers.
			 *
			 * CNTHCTL_EL2.EL1PCTEN: Set to one to disable traps to
			 *  Hyp mode of  Non-secure EL0 and EL1 accesses to the
			 *  physical counter registers.
			 */
			write_cnthctl_el2(CNTHCTL_RESET_VAL |
						EL1PCEN_BIT | EL1PCTEN_BIT);

			/*
			 * Initialise CNTVOFF_EL2 to zero as it resets to an
			 * architecturally UNKNOWN value.
			 */
			write_cntvoff_el2(0);

			/*
			 * Set VPIDR_EL2 and VMPIDR_EL2 to match MIDR_EL1 and
			 * MPIDR_EL1 respectively.
			 */
			write_vpidr_el2(read_midr_el1());
			write_vmpidr_el2(read_mpidr_el1());

			/*
			 * Initialise VTTBR_EL2. All fields are architecturally
			 * UNKNOWN on reset.
			 *
			 * VTTBR_EL2.VMID: Set to zero. Even though EL1&0 stage
			 *  2 address translation is disabled, cache maintenance
			 *  operations depend on the VMID.
			 *
			 * VTTBR_EL2.BADDR: Set to zero as EL1&0 stage 2 address
			 *  translation is disabled.
			 */
			write_vttbr_el2(VTTBR_RESET_VAL &
				~((VTTBR_VMID_MASK << VTTBR_VMID_SHIFT)
				| (VTTBR_BADDR_MASK << VTTBR_BADDR_SHIFT)));

			/*
			 * Initialise MDCR_EL2, setting all fields rather than
			 * relying on hw. Some fields are architecturally
			 * UNKNOWN on reset.
			 *
			 * MDCR_EL2.HLP: Set to one so that event counter
			 *  overflow, that is recorded in PMOVSCLR_EL0[0-30],
			 *  occurs on the increment that changes
			 *  PMEVCNTR<n>_EL0[63] from 1 to 0, when ARMv8.5-PMU is
			 *  implemented. This bit is RES0 in versions of the
			 *  architecture earlier than ARMv8.5, setting it to 1
			 *  doesn't have any effect on them.
			 *
			 * MDCR_EL2.TTRF: Set to zero so that access to Trace
			 *  Filter Control register TRFCR_EL1 at EL1 is not
			 *  trapped to EL2. This bit is RES0 in versions of
			 *  the architecture earlier than ARMv8.4.
			 *
			 * MDCR_EL2.HPMD: Set to one so that event counting is
			 *  prohibited at EL2. This bit is RES0 in versions of
			 *  the architecture earlier than ARMv8.1, setting it
			 *  to 1 doesn't have any effect on them.
			 *
			 * MDCR_EL2.TPMS: Set to zero so that accesses to
			 *  Statistical Profiling control registers from EL1
			 *  do not trap to EL2. This bit is RES0 when SPE is
			 *  not implemented.
			 *
			 * MDCR_EL2.TDRA: Set to zero so that Non-secure EL0 and
			 *  EL1 System register accesses to the Debug ROM
			 *  registers are not trapped to EL2.
			 *
			 * MDCR_EL2.TDOSA: Set to zero so that Non-secure EL1
			 *  System register accesses to the powerdown debug
			 *  registers are not trapped to EL2.
			 *
			 * MDCR_EL2.TDA: Set to zero so that System register
			 *  accesses to the debug registers do not trap to EL2.
			 *
			 * MDCR_EL2.TDE: Set to zero so that debug exceptions
			 *  are not routed to EL2.
			 *
			 * MDCR_EL2.HPME: Set to zero to disable EL2 Performance
			 *  Monitors.
			 *
			 * MDCR_EL2.TPM: Set to zero so that Non-secure EL0 and
			 *  EL1 accesses to all Performance Monitors registers
			 *  are not trapped to EL2.
			 *
			 * MDCR_EL2.TPMCR: Set to zero so that Non-secure EL0
			 *  and EL1 accesses to the PMCR_EL0 or PMCR are not
			 *  trapped to EL2.
			 *
			 * MDCR_EL2.HPMN: Set to value of PMCR_EL0.N which is the
			 *  architecturally-defined reset value.
			 */
			mdcr_el2 = ((MDCR_EL2_RESET_VAL | MDCR_EL2_HLP |
					MDCR_EL2_HPMD) |
					((read_pmcr_el0() & PMCR_EL0_N_BITS)
					>> PMCR_EL0_N_SHIFT)) &
					~(MDCR_EL2_TTRF | MDCR_EL2_TPMS |
					MDCR_EL2_TDRA_BIT | MDCR_EL2_TDOSA_BIT |
					MDCR_EL2_TDA_BIT | MDCR_EL2_TDE_BIT |
					MDCR_EL2_HPME_BIT | MDCR_EL2_TPM_BIT |
					MDCR_EL2_TPMCR_BIT);

			write_mdcr_el2(mdcr_el2);

			/*
			 * Initialise HSTR_EL2. All fields are architecturally
			 * UNKNOWN on reset.
			 *
			 * HSTR_EL2.T<n>: Set all these fields to zero so that
			 *  Non-secure EL0 or EL1 accesses to System registers
			 *  do not trap to EL2.
			 */
			write_hstr_el2(HSTR_EL2_RESET_VAL & ~(HSTR_EL2_T_MASK));
			/*
			 * Initialise CNTHP_CTL_EL2. All fields are
			 * architecturally UNKNOWN on reset.
			 *
			 * CNTHP_CTL_EL2:ENABLE: Set to zero to disable the EL2
			 *  physical timer and prevent timer interrupts.
			 */
			write_cnthp_ctl_el2(CNTHP_CTL_RESET_VAL &
						~(CNTHP_CTL_ENABLE_BIT));
		}
		enable_extensions_nonsecure(el2_unused);
	}

	cm_el1_sysregs_context_restore(security_state);
	cm_set_next_eret_context(security_state);
}
