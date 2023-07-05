/*
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <lib/el3_runtime/aarch64/context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/sve.h>
#include <plat/common/platform.h>
#include <lib/extensions/simd_ctx.h>
#include <lib/el3_runtime/cpu_data.h>
#include <common/debug.h>

#if CTX_INCLUDE_SIMD_REGS
__section(".bss.cpu_context.simd")
static simd_regs_t simd_context[NON_SECURE + 1][PLATFORM_CORE_COUNT];
#endif

void simd_ctx_switch(enum simd_ctx_op op, uint32_t security_state)
{
	uint32_t idx = get_cpu_context_index(security_state);
	if (idx > NON_SECURE) {
		ERROR("simd_ctx: invalid context. RME not supported. aborting\n");
		panic();
	}

#if CTX_INCLUDE_SVE_REGS
	if (op == SIMD_CTX_OP_SAVE)
		sve_context_save((&simd_context[security_state][plat_my_core_pos()]);
	else
		sve_context_restore(&simd_context[security_state][plat_my_core_pos()]);
#elif CTX_INCLUDE_FPREGS
	if (op == SIMD_CTX_OP_SAVE)
		fpregs_context_save((fp_regs_t *)&simd_context[security_state][plat_my_core_pos()]);
	else
		fpregs_context_restore((fp_regs_t *)&simd_context[security_state][plat_my_core_pos()]);
#endif
}
