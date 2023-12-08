/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Dispatch synchronous system register traps from lower ELs.
 */

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

void inject_undef64(cpu_context_t *ctx)
{
	u_register_t spsr_el3 = read_spsr_el3();
	unsigned int level = (unsigned int)GET_EL(spsr_el3);
	u_register_t esr = (EC_UNKNOWN << ESR_EC_SHIFT) | ESR_IL_BIT;
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t elr_el3;

	spsr_el3 |= SPSR_DAIF_MASK << SPSR_DAIF_SHIFT;

	if (level == MODE_EL2) {
		elr_el3 = read_vbar_el2() + 0x200; /* Sync exception address */
		write_esr_el2(esr);
		write_elr_el2(read_elr_el3());

	} else {
		elr_el3 = read_vbar_el1() + 0x200; /* Sync exception address */
		write_esr_el1(esr);
		write_elr_el1(read_elr_el3());
	}

	write_ctx_reg(state, CTX_SPSR_EL3, spsr_el3);
	write_ctx_reg(state, CTX_ELR_EL3, elr_el3);
	return;
}
