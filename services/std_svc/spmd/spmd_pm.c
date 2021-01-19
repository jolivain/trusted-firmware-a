/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <lib/el3_runtime/context_mgmt.h>
#include "spmd_private.h"

static struct {
	bool secondary_ep_locked;
	uintptr_t secondary_ep;
	/* TODO: add lock */
} g_spmd_pm;

/*******************************************************************************
 * spmd_build_spmc_message
 *
 * Builds an SPMD to SPMC direct message request.
 ******************************************************************************/
static void spmd_build_spmc_message(gp_regs_t *gpregs, unsigned long long message)
{
	write_ctx_reg(gpregs, CTX_GPREG_X0, FFA_MSG_SEND_DIRECT_REQ_SMC32);
	write_ctx_reg(gpregs, CTX_GPREG_X1,
		(SPMD_DIRECT_MSG_ENDPOINT_ID << FFA_DIRECT_MSG_SOURCE_SHIFT) |
		spmd_spmc_id_get());
	write_ctx_reg(gpregs, CTX_GPREG_X2, FFA_PARAM_MBZ);
	write_ctx_reg(gpregs, CTX_GPREG_X3, message);
}

/*******************************************************************************
 * spmd_pm_secondary_ep_register
 ******************************************************************************/
int spmd_pm_secondary_ep_register(uintptr_t entry_point)
{
	if (g_spmd_pm.secondary_ep_locked == true) {
		return -EINVAL;
	}

	/*
	 * Check entry_point address is a PA within
	 * load_address <= entry_point < load_address + binary_size
	 */
	if (!spmd_check_address_in_binary_image(entry_point)) {
		ERROR("%s entry point is not within image boundaries\n",
			__func__);
		return -EINVAL;
	}

	g_spmd_pm.secondary_ep = entry_point;
	g_spmd_pm.secondary_ep_locked = true;

	VERBOSE("%s %lx\n", __func__, entry_point);

	return 0;
}

/*******************************************************************************
 * This CPU has been turned on. Enter SPMC to initialise S-EL1 or S-EL2. As part
 * of the SPMC initialization path, they will initialize any SPs that they
 * manage. Entry into SPMC is done after initialising minimal architectural
 * state that guarantees safe execution.
 ******************************************************************************/
static void spmd_cpu_on_finish_handler(u_register_t unused)
{
	entry_point_info_t *spmc_ep_info = spmd_spmc_ep_info_get();
	spmd_spm_core_context_t *ctx = spmd_get_context();
	unsigned int linear_id = plat_my_core_pos();
	uint64_t rc;

	assert(ctx != NULL);
	assert(ctx->state != SPMC_STATE_ON);
	assert(spmc_ep_info != NULL);

	/*
	 * TODO: this might require locking the spmc_ep_info structure,
	 * or provisioning one structure per cpu
	 */
	if (g_spmd_pm.secondary_ep == 0UL) {
		goto exit;
	}

	spmc_ep_info->pc = g_spmd_pm.secondary_ep;
	cm_setup_context(&ctx->cpu_ctx, spmc_ep_info);
	//write_ctx_reg(get_gpregs_ctx(&ctx->cpu_ctx), CTX_GPREG_X0, linear_id);
	/* TODO: clear other GP regs? */

	/* Mark CPU as initiating ON operation */
	ctx->state = SPMC_STATE_ON_PENDING;

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc != 0ULL) {
		ERROR("%s failed (%llu) on CPU%u\n", __func__, rc,
			linear_id);
		ctx->state = SPMC_STATE_OFF;
		return;
	}

exit:
	ctx->state = SPMC_STATE_ON;

	VERBOSE("CPU %u on!\n", linear_id);
}

/*******************************************************************************
 * spmd_cpu_off_handler
 ******************************************************************************/
static int32_t spmd_cpu_off_handler(u_register_t unused)
{
	spmd_spm_core_context_t *ctx = spmd_get_context();
	unsigned int linear_id = plat_my_core_pos();
	int64_t rc;

	assert(ctx != NULL);
	assert(ctx->state != SPMC_STATE_OFF);

	if (g_spmd_pm.secondary_ep == 0UL) {
		goto exit;
	}

	/* Build an SPMD to SPMC direct message request. */
	spmd_build_spmc_message(get_gpregs_ctx(&ctx->cpu_ctx), PSCI_CPU_OFF);

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc != 0ULL) {
		ERROR("%s failed (%llu) on CPU%u\n", __func__, rc, linear_id);
	}

	/* TODO expect FFA_DIRECT_MSG_RESP returned from SPMC */

exit:
	ctx->state = SPMC_STATE_OFF;

	VERBOSE("CPU %u off!\n", linear_id);

	return 0;
}

/*******************************************************************************
 * Structure populated by the SPM Dispatcher to perform any bookkeeping before
 * PSCI executes a power mgmt. operation.
 ******************************************************************************/
const spd_pm_ops_t spmd_pm = {
	.svc_on_finish = spmd_cpu_on_finish_handler,
	.svc_off = spmd_cpu_off_handler
};
