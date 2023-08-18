/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/cassert.h>
#include <lib/el3_runtime/pubsub.h>
#include <lib/extensions/sve.h>

CASSERT(SVE_VECTOR_LEN <= 2048, assert_sve_vl_too_long);
CASSERT(SVE_VECTOR_LEN >= 128, assert_sve_vl_too_short);
CASSERT((SVE_VECTOR_LEN % 128) == 0, assert_sve_vl_granule);

/*
 * Converts SVE vector size restriction in bytes to LEN according to ZCR_EL3 documentation.
 * VECTOR_SIZE = (LEN+1) * 128
 */
#define CONVERT_SVE_LENGTH(x)	(((x / 128) - 1))

void sve_enable(global_context_t *global_ctx)
{
	u_register_t cptr_el3;
	global_el3_state_t *global_el3state_ctx = get_global_el3state_ctx_from_global_ctx(global_ctx);

	cptr_el3 = read_ctx_reg(global_el3state_ctx, CTX_CPTR_EL3);
	cptr_el3 = (cptr_el3 | CPTR_EZ_BIT) & ~(TFP_BIT);
	write_ctx_reg(global_el3state_ctx, CTX_CPTR_EL3, cptr_el3);
	write_ctx_reg(global_el3state_ctx, CTX_ZCR_EL3,
		(ZCR_EL3_LEN_MASK & CONVERT_SVE_LENGTH(SVE_VECTOR_LEN)));
}

void sve_init_el2_unused(void)
{
	/*
	 * CPTR_EL2.TFP: Set to zero so that Non-secure accesses to Advanced
	 *  SIMD and floating-point functionality from both Execution states do
	 *  not trap to EL2.
	 */
	write_cptr_el2(read_cptr_el2() & ~CPTR_EL2_TFP_BIT);
}

void sve_disable(global_context_t *global_ctx)
{
	u_register_t reg;
	global_el3_state_t *global_el3state_ctx = get_global_el3state_ctx_from_global_ctx(global_ctx);

	reg = read_ctx_reg(global_el3state_ctx, CTX_CPTR_EL3);
	reg &= ~CPTR_EZ_BIT;	/* Trap SVE */
	reg |= TFP_BIT;		/* Trap FPU/SIMD */
	write_ctx_reg(global_el3state_ctx, CTX_CPTR_EL3, reg);
}
