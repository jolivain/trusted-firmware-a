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

void sve_enable_global(global_context_t *global_ctx)
{
	u_register_t cptr_el3;

	/* Enable access to SVE functionality for all ELs. */
	cptr_el3 = global_ctx->ctx_cptr_el3;
	cptr_el3 = (cptr_el3 | CPTR_EZ_BIT) & ~(TFP_BIT);
	global_ctx->ctx_cptr_el3 = cptr_el3;

	/* Restrict maximum SVE vector length (SVE_VECTOR_LEN+1) * 128. */
	global_ctx->ctx_zcr_el3 = (ZCR_EL3_LEN_MASK & CONVERT_SVE_LENGTH(SVE_VECTOR_LEN));
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

void sve_disable_global(global_context_t *global_ctx)
{
	u_register_t reg;

	/* Disable SVE and FPU since they share registers. */
	reg = global_ctx->ctx_cptr_el3;
	reg &= ~CPTR_EZ_BIT;	/* Trap SVE */
	reg |= TFP_BIT;		/* Trap FPU/SIMD */
	global_ctx->ctx_cptr_el3 = reg;
}
