/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Dispatch synchronous system register traps from lower ELs.
 */

#include <arch_features.h>
#include <arch_helpers.h>
#include <bl31/sync_handle.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>

int handle_sysreg_trap(uint64_t esr_el3, cpu_context_t *ctx)
{
	uint64_t __unused opcode = esr_el3 & ISS_SYSREG_OPCODE_MASK;

#if ENABLE_FEAT_RNG_TRAP
	if ((opcode == ISS_SYSREG_OPCODE_RNDR) || (opcode == ISS_SYSREG_OPCODE_RNDRRS)) {
		return plat_handle_rng_trap(esr_el3, ctx);
	}
#endif

#if IMPDEF_SYSREG_TRAP
	if ((opcode & ISS_SYSREG_OPCODE_IMPDEF) == ISS_SYSREG_OPCODE_IMPDEF) {
		return plat_handle_impdef_trap(esr_el3, ctx);
	}
#endif

	return TRAP_RET_UNHANDLED;
}

static bool is_tge_enabled(void)
{
	u_register_t hcr_el2 = read_hcr_el2();

	return ((read_feat_vhe_id_field() != 0U) && (hcr_el2 & HCR_TGE_BIT));
}

static unsigned int target_el(void)
{
	unsigned int level = (unsigned int)GET_EL(read_spsr_el3());

	if (level > MODE_EL1) {
		return level;
	} else if (is_tge_enabled()) {
		return MODE_EL2;
	} else {
		return MODE_EL1;
	}
}

static u_register_t create_spsr(u_register_t old_spsr, unsigned int target_el)
{
	u_register_t new_spsr = 0;
	u_register_t sctlr, gcscr;

	/* Get sctlr based on target EL for later usage */
	if (target_el == MODE_EL2) {
		sctlr = read_sctlr_el2();
	} else {
		sctlr = read_sctlr_el1();
	}

	/* Explictily create all bits based on psuedocode AArch64.TakeException */

	/* Set M bits for target EL in AArch64 mode */
	if (target_el == MODE_EL2) {
		new_spsr |= (SPSR_M_AARCH64 << SPSR_M_SHIFT) | SPSR_M_EL2H;
	} else {
		new_spsr |= (SPSR_M_AARCH64 << SPSR_M_SHIFT) | SPSR_M_EL1H;
	}

	/* Mask all exceptions, update DAIF bits */
	new_spsr |= SPSR_DAIF_MASK << SPSR_DAIF_SHIFT;

	/* If FEAT_BTI is present, clear BTYPE bits */
	new_spsr |= old_spsr & (SPSR_BTYPE_MASK_AARCH64 << SPSR_BTYPE_SHIFT_AARCH64);
	if (is_armv8_5_bti_present()) {
		new_spsr &= ~(SPSR_BTYPE_MASK_AARCH64 << SPSR_BTYPE_SHIFT_AARCH64);
	}

	/* If SSBS is implemented, take the value from SCTLR.DSSBS */
	new_spsr |= old_spsr & SPSR_SSBS_BIT_AARCH64;
	if (is_feat_ssbs_present()) {
		if (sctlr & SCTLR_DSSBS_BIT) {
			new_spsr |= SPSR_SSBS_BIT_AARCH64;
		} else {
			new_spsr &= ~SPSR_SSBS_BIT_AARCH64;
		}
	}

	/* If FEAT_NMI is implemented, ALLINT = !(SCTLR.SPINTMASK) */
	new_spsr |= old_spsr & SPSR_ALLINT_BIT_AARCH64;
	if (is_feat_nmi_present()) {
		if (sctlr & SCTLR_SPINTMASK_BIT) {
			new_spsr &= ~SPSR_ALLINT_BIT_AARCH64;
		} else {
			new_spsr |= SPSR_ALLINT_BIT_AARCH64;
		}
	}

	/* Clear PSTATE.IL bit explicitly */
	new_spsr &= ~SPSR_IL_BIT;

	/* Clear PSTATE.SS bit explicitly */
	new_spsr &= ~SPSR_SS_BIT;

	/* Update PSTATE.PAN bit */
	new_spsr |= old_spsr & SPSR_PAN_BIT;
	if (is_feat_pan_supported() &&
	    ((target_el == MODE_EL1) || is_tge_enabled()) &&
	    ((sctlr & SCTLR_SPAN_BIT) == 0U)) {
	    new_spsr |= SPSR_PAN_BIT;
	}

	/* Clear UAO bit if FEAT_UAO is present */
	new_spsr |= old_spsr & SPSR_UAO_BIT_AARCH64;
	if (is_armv8_2_uao_present()) {
		new_spsr &= ~(SPSR_UAO_BIT_AARCH64);
	}

	/* DIT bits are unchanged */
	new_spsr |= old_spsr & SPSR_DIT_BIT;

	/* If FEAT_MTE2 is implemented mask tag faults by setting TCO bit */
	new_spsr |= old_spsr & SPSR_TCO_BIT_AARCH64;
	if (get_armv8_5_mte_support() >= MTE_IMPLEMENTED_ELX) {
		new_spsr |= SPSR_TCO_BIT_AARCH64;
	}

	/* NZCV bits are unchanged */
	new_spsr |= old_spsr & (SPSR_NZCV);

	/* If FEAT_EBEP is present set PM bit */
	new_spsr |= old_spsr & SPSR_PM_BIT_AARCH64;
	if (is_feat_ebep_present()) {
		new_spsr |= SPSR_PM_BIT_AARCH64;
	}

	/* If FEAT_SEBEP is present clear PPEND bit */
	new_spsr |= old_spsr & SPSR_PPEND_BIT;
	if (is_feat_sebep_present()) {
		new_spsr |= SPSR_PPEND_BIT;
	}

	/* If FEAT_GCS is supported, update EXLOCK bit */
	new_spsr |= old_spsr & SPSR_EXLOCK_BIT_AARCH64;
	if (is_feat_gcs_supported()) {
		if (target_el == MODE_EL2) {
			gcscr = read_gcscr_el2();
		} else {
			gcscr = read_gcscr_el1();
		}
		new_spsr |= (gcscr & GCSCR_EXLOCK_EN_BIT);
	}

	return new_spsr;
}

void inject_undef64(cpu_context_t *ctx)
{
	u_register_t esr = (EC_UNKNOWN << ESR_EC_SHIFT) | ESR_IL_BIT;
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t elr_el3 = read_ctx_reg(state, CTX_ELR_EL3);
	u_register_t old_spsr = read_ctx_reg(state, CTX_SPSR_EL3);
	unsigned int to_el = target_el();
	u_register_t new_spsr = 0;

	if (to_el == MODE_EL2) {
		write_elr_el2(elr_el3);
		elr_el3 = read_vbar_el2() + 0x200; /* Sync exception address */
		write_esr_el2(esr);
		write_spsr_el2(old_spsr);
	} else {
		write_elr_el1(elr_el3);
		elr_el3 = read_vbar_el1() + 0x200; /* Sync exception address */
		write_esr_el1(esr);
		write_spsr_el1(old_spsr);
	}

	new_spsr = create_spsr(old_spsr, to_el);

	write_ctx_reg(state, CTX_SPSR_EL3, new_spsr);
	write_ctx_reg(state, CTX_ELR_EL3, elr_el3);
}
