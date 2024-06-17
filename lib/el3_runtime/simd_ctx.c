/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <lib/el3_runtime/aarch64/context.h>
#include <lib/el3_runtime/context_mgmt.h>
/* drop this */
#include <lib/extensions/sve.h>
#include <plat/common/platform.h>
#include <lib/el3_runtime/simd_ctx.h>
#include <lib/el3_runtime/cpu_data.h>
#include <common/debug.h>

#if CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS
__section(".bss.cpu_context.simd")
static simd_regs_t simd_context[NON_SECURE + 1][PLATFORM_CORE_COUNT];
#endif /* CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS */

void simd_ctx_save(uint32_t security_state, bool hint_sve)
{
	simd_regs_t *regs;

	if (security_state != NON_SECURE && security_state != SECURE) {
		ERROR("simd_ctx: invalid context. RME not supported. aborting\n");
		panic();
	}

	regs = &simd_context[security_state][plat_my_core_pos()];

#if CTX_INCLUDE_SVE_REGS
	regs->hint = hint_sve;

	if (hint_sve) {
		/*
		 * Hint bit denoting absence of SVE live state. Hence, only
		 * save FP context.
		 */
		fpregs_context_save(regs);
	} else {
		sve_context_save(regs);
	}
#elif CTX_INCLUDE_FPREGS
	fpregs_context_save(regs);
#endif
}

void simd_ctx_restore(uint32_t security_state)
{
	simd_regs_t *regs;

	if (security_state != NON_SECURE && security_state != SECURE) {
		ERROR("simd_ctx: invalid context. RME not supported. aborting\n");
		panic();
	}

	regs = &simd_context[security_state][plat_my_core_pos()];

#if CTX_INCLUDE_SVE_REGS
	if (regs->hint) {
		fpregs_context_restore(regs);
	} else {
		sve_context_restore(regs);
	}
#elif CTX_INCLUDE_FPREGS
	fpregs_context_restore(regs);
#endif
}
