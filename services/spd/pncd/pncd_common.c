/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <string.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>

#include "pncd_private.h"

/*******************************************************************************
 * Given a secure payload entrypoint info pointer, entry point PC, register
 * width, cpu id & pointer to a context data structure, this function will
 * initialize pnc context and entry point info for the secure payload
 ******************************************************************************/
void pncd_init_pnc_ep_state(struct entry_point_info *pnc_entry_point,
				uint32_t rw,
				uint64_t pc,
				pnc_context_t *pnc_ctx)
{
	uint32_t ep_attr;

	/* Passing a NULL context is a critical programming error */
	assert(pnc_ctx);
	assert(pnc_entry_point);
	assert(pc);

	/*
	 * We support AArch64 PNC for now.
	 * TODO: Add support for AArch32 PNC
	 */
#ifdef EL1S_32
	assert(rw == PNC_AARCH32);
#else
	assert(rw == PNC_AARCH64);
#endif

	/* Associate this context with the cpu specified */
	pnc_ctx->mpidr = read_mpidr_el1();
	pnc_ctx->state = 0;
	set_pnc_pstate(pnc_ctx->state, PNC_PSTATE_OFF);
	clr_std_smc_active_flag(pnc_ctx->state);

	cm_set_context(&pnc_ctx->cpu_ctx, SECURE);

	/* initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if (read_sctlr_el3() & SCTLR_EE_BIT) {
		ep_attr |= EP_EE_BIG;
	}
	SET_PARAM_HEAD(pnc_entry_point, PARAM_EP, VERSION_1, ep_attr);

	pnc_entry_point->pc = pc;
#ifdef EL1S_32
	pnc_entry_point->spsr = SPSR_MODE32(MODE32_svc,
					SPSR_T_ARM,
					SPSR_E_LITTLE,
					DISABLE_ALL_EXCEPTIONS);
#else
	pnc_entry_point->spsr = SPSR_64(MODE_EL1,
					MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS);
#endif
	memset(&pnc_entry_point->args, 0, sizeof(pnc_entry_point->args));
}

/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Applies the S-EL1 system register context from pnc_ctx->cpu_ctx.
 * 2. Saves the current C runtime state (callee saved registers) on the stack
 *    frame and saves a reference to this state.
 * 3. Calls el3_exit() so that the EL3 system and general purpose registers
 *    from the pnc_ctx->cpu_ctx are used to enter the secure payload image.
 ******************************************************************************/
uint64_t pncd_synchronous_sp_entry(pnc_context_t *pnc_ctx)
{
	uint64_t rc;

	assert(pnc_ctx != NULL);
	assert(pnc_ctx->c_rt_ctx == 0);

	/* Apply the Secure EL1 system register context and switch to it */
	assert(cm_get_context(SECURE) == &pnc_ctx->cpu_ctx);
	cm_el1_sysregs_context_restore(SECURE);
#if CTX_EAGER_SAVE_FPREGS
	fpregs_context_restore(get_fpregs_ctx(cm_get_context(SECURE)));
#endif
	cm_set_next_eret_context(SECURE);

	rc = pncd_enter_sp(&pnc_ctx->c_rt_ctx);
#if DEBUG
	pnc_ctx->c_rt_ctx = 0;
#endif
	return rc;
}


/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Saves the S-EL1 system register context tp pnc_ctx->cpu_ctx.
 * 2. Restores the current C runtime state (callee saved registers) from the
 *    stack frame using the reference to this state saved in pncd_enter_sp().
 * 3. It does not need to save any general purpose or EL3 system register state
 *    as the generic smc entry routine should have saved those.
 ******************************************************************************/
void pncd_synchronous_sp_exit(pnc_context_t *pnc_ctx, uint64_t ret)
{
	assert(pnc_ctx != NULL);
	/* Save the Secure EL1 system register context */
	assert(cm_get_context(SECURE) == &pnc_ctx->cpu_ctx);
	cm_el1_sysregs_context_save(SECURE);
#if CTX_EAGER_SAVE_FPREGS
	fpregs_context_save(get_fpregs_ctx(cm_get_context(SECURE)));
#endif

	assert(pnc_ctx->c_rt_ctx != 0);
	pncd_exit_sp(pnc_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}
