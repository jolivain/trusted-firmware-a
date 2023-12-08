/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Dispatch synchronous system register traps from lower ELs.
 */

#include <arch_helpers.h>
#include <arch_features.h>
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

static bool is_el2_host()
{
	u_register_t hcr_el2 = read_hcr_el2();

	return 	((read_feat_vhe_id_field() != 0U) &&
		 (hcr_el2 & HCR_TGE_BIT) &&
		 (hcr_el2 & HCR_E2H_BIT));
}

static unsigned int target_el(void)
{
	unsigned int level = (unsigned int)GET_EL(read_spsr_el3());
	if (level == MODE_EL0) {
		if (is_el2_host()) {
			level = MODE_EL2;
		} else {
			level = MODE_EL1;
		}
	}
	return level;
}

void inject_undef64(cpu_context_t *ctx)
{
	u_register_t esr = (EC_UNKNOWN << ESR_EC_SHIFT) | ESR_IL_BIT;
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t elr_el3 = read_ctx_reg(state, CTX_ELR_EL3);
	u_register_t old_spsr = read_ctx_reg(state, CTX_SPSR_EL3);
	unsigned int to_el = target_el();
	u_register_t new_spsr = 0;
	u_register_t sctlr;

	/* Recreate PSTATE for target EL */

	/* NZCV bits are unchanged */
	new_spsr |= old_spsr & (SPSR_NZCV);
	/* Mask exceptions */
	new_spsr |= SPSR_DAIF_MASK << SPSR_DAIF_SHIFT;

	if (to_el == MODE_EL2) {
		new_spsr |= old_spsr & SPSR_M_EL2H;
		sctlr = read_sctlr_el2();
		write_elr_el2(elr_el3);
		elr_el3 = read_vbar_el2() + 0x200; /* Sync exception address */
		write_esr_el2(esr);
		write_spsr_el2(old_spsr);
	} else {
		new_spsr |= old_spsr & SPSR_M_EL1H;
		sctlr = read_sctlr_el1();
		write_elr_el1(elr_el3);
		elr_el3 = read_vbar_el1() + 0x200; /* Sync exception address */
		write_esr_el1(esr);
		write_spsr_el1(old_spsr);
	}

	/* Set PSTATE.PAN bit */
	new_spsr |= old_spsr & SPSR_PAN_BIT;
	if (is_feat_pan_supported() &&
	    ((to_el == MODE_EL1) || is_el2_host()) &&
	    ((sctlr & SCTLR_SPAN_BIT) == 0U)) {
		new_spsr |= SPSR_PAN_BIT;
	}

	/* If SSBS is implemented, take the value from SCTLR.DSSBS */
	new_spsr |= old_spsr & SPSR_SSBS_BIT_AARCH64;
	u_register_t sbss_enabled = (read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_SSBS_SHIFT)
				    & ID_AA64PFR1_EL1_SSBS_MASK;
	if (sbss_enabled != SSBS_UNAVAILABLE) {
		new_spsr |= ((sctlr & SCTLR_DSSBS_BIT) >> SCTLR_DSSBS_SHIFT)
			     << SPSR_SSBS_SHIFT_AARCH64;
	}

	/* If FEAT_MTE2 is implemented mask tag faults by setting TCO */
	new_spsr |= old_spsr & SPSR_TCO_BIT_AARCH64;
	if (get_armv8_5_mte_support() >= MTE_IMPLEMENTED_ELX) {
		new_spsr |= SPSR_TCO_BIT_AARCH64;
	}

	/* If FEAT_BTI is present, disable branch target exception */
	new_spsr |= old_spsr & (SPSR_BTYPE_MASK_AARCH64 << SPSR_BTYPE_SHIFT_AARCH64);
	if (is_armv8_5_bti_present()) {
		new_spsr &= ~(SPSR_BTYPE_MASK_AARCH64 << SPSR_BTYPE_SHIFT_AARCH64);
	}

	/* DIT bits are unchanged */
	new_spsr |= old_spsr & SPSR_DIT_BIT;

	/* UAO bits needs to be re-created here */

	/* TODO: Re-create other bits as per TF-A feature awareness */

	write_ctx_reg(state, CTX_SPSR_EL3, new_spsr);
	write_ctx_reg(state, CTX_ELR_EL3, elr_el3);
	return;
}
