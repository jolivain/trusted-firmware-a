/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <context.h>
#include <stdbool.h>

#include <bl31/sync_handle.h>

#define RNDR	"S3_3_C2_C4_0"

#define ISS_SYSREG_OPCODE_RNDR		0x30c809U
#define ISS_SYSREG_OPCODE_RNDRSS	0x32c809U

/*
 * SCR_EL3.SCR_TRNDR_BIT also affects execution in EL3, so allow to disable
 * the trap temporarily.
 */
static void enable_rng_trap(bool enable)
{
	uint64_t scr_el3 = read_scr_el3();

	if (enable) {
		scr_el3 |= SCR_TRNDR_BIT;
	} else {
		scr_el3 &= ~SCR_TRNDR_BIT;
	}

	write_scr_el3(scr_el3);
}

int handle_sysreg_trap(uint64_t esr_el3, cpu_context_t *ctx)
{
	unsigned long rnd;

	if ((esr_el3 & ISS_SYSREG_OPCODE_MASK) == ISS_SYSREG_OPCODE_RNDR ||
	    (esr_el3 & ISS_SYSREG_OPCODE_MASK) == ISS_SYSREG_OPCODE_RNDRSS) {
		unsigned int rt = get_sysreg_iss_rt(esr_el3);

		if (rt != 31 && !is_sysreg_iss_write(esr_el3)) {
			enable_rng_trap(false);
			__asm__ volatile ("mrs %0, " RNDR "\n" : "=r" (rnd));
			enable_rng_trap(true);

			ctx->gpregs_ctx.ctx_regs[rt] = rnd;
		}

		return TRAP_RET_SKIP;
	}

	return TRAP_RET_UNHANDLED;
}
