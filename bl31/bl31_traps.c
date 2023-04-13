/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Dispatch synchronous system register traps from lower ELs.
 */

#include <bl31/sync_handle.h>
#include <common/debug.h>
#include <context.h>

int handle_sysreg_trap(uint64_t esr_el3, cpu_context_t *ctx)
{
	uint64_t opcode = esr_el3 & ISS_SYSREG_OPCODE_MASK;

	INFO("esr = 0x%lx, opcode = 0x%lx\n", esr_el3, esr_el3 & ISS_SYSREG_OPCODE_MASK);

#if ENABLE_FEAT_RNG_TRAP
	if ((opcode == ISS_SYSREG_OPCODE_RNDR) || (opcode == ISS_SYSREG_OPCODE_RNDRRS)) {
		return plat_handle_rng_trap(esr_el3, ctx);
	}
#endif

#if ENABLE_FEAT_MSR_TRAP
	if ((opcode & ISS_SYSREG_OPCODE_IMPDEF) == ISS_SYSREG_OPCODE_IMPDEF) {
		return plat_handle_msr_trap(esr_el3, ctx);
	}
#endif

	return TRAP_RET_UNHANDLED;
}
