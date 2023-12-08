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

static bool is_el2_as_host(void)
{
	u_register_t hcr = read_hcr_el2();
	return (read_feat_vhe_id_field() != 0U &&
	       (hcr & HCR_TGE_BIT) && (hcr & HCR_E2H_BIT));
}

void inject_undef64(cpu_context_t *ctx)
{
	u_register_t esr = (EC_UNKNOWN << ESR_EC_SHIFT) | ESR_IL_BIT;
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t elr_el3 = read_ctx_reg(state, CTX_ELR_EL3);
	u_register_t spsr_el3 = read_ctx_reg(state, CTX_SPSR_EL3);
	unsigned int level = (unsigned int)GET_EL(spsr_el3);

	switch (level) {
	case MODE_EL2:
		write_elr_el2(elr_el3);
		elr_el3 = read_vbar_el2() + 0x200; /* Sync exception address */
		write_esr_el2(esr);
		break;
	case MODE_EL1:
		write_elr_el1(elr_el3);
		elr_el3 = read_vbar_el1() + 0x200; /* Sync exception address */
		write_esr_el1(esr);
		break;
	case MODE_EL0:
		u_register_t sctlr;
		if (is_el2_as_host()) {
			spsr_el3 |= SPSR_M_EL2H;
			write_elr_el2(elr_el3);
			elr_el3 = read_vbar_el2() + 0x200; /* Sync exception address */
			write_esr_el2(esr);
			sctlr = read_sctlr_el2();
		} else {
			spsr_el3 |= SPSR_M_EL1H;
			write_elr_el1(elr_el3);
			elr_el3 = read_vbar_el1() + 0x200; /* Sync exception address */
			write_esr_el1(esr);
			sctlr = read_sctlr_el1();
		}
		/* Set PSTAT.PAN bit as the target exception level is EL1 or EL2 */
		if (is_feat_pan_supported() && ((sctlr & SCTLR_SPAN_BIT) == 0U)) {
			spsr_el3 |= SPSR_PAN_BIT;
		}
		/* If SSBS is implemented, take the value from SCTLR.DSSBS */
		u_register_t sbss_enabled = (read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_SSBS_SHIFT)
					    & ID_AA64PFR1_EL1_SSBS_MASK;
		if (sbss_enabled != SSBS_UNAVAILABLE) {
			spsr_el3 |= ((sctlr & SCTLR_DSSBS_BIT) >> SCTLR_DSSBS_SHIFT)
				     << SPSR_SSBS_SHIFT_AARCH64;
		}
		break;
	}

	/* Mask exceptions */
	spsr_el3 |= SPSR_DAIF_MASK << SPSR_DAIF_SHIFT;
	/* If FEAT_MTE2 is implemented mask tag faults by setting TCO */
	if (get_armv8_5_mte_support() >= MTE_IMPLEMENTED_ELX) {
		spsr_el3 |= SPSR_TCO_BIT_AARCH64;
	}
	/* If FEAT_BTI is present, disable branch target exception */
	if (is_armv8_5_bti_present()) {
		spsr_el3 &= ~(SPSR_BTYPE_MASK_AARCH64 << SPSR_BTYPE_SHIFT_AARCH64);
	}
	write_ctx_reg(state, CTX_SPSR_EL3, spsr_el3);
	write_ctx_reg(state, CTX_ELR_EL3, elr_el3);
	return;
}
