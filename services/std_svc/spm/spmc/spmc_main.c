/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <errno.h>

#include <bl31/bl31.h>
#include <bl31/ehf.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/smccc.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <services/ffa_svc.h>
#include <services/spmc_svc.h>

#include "spm_common.h"

static sp_context_t sp_ctx;
static void *spmc_manifest;

static int32_t sp_init(void)
{
	uint64_t rc;
	sp_context_t *ctx;

	INFO("Secure Partition init start.\n");

	ctx = &sp_ctx;
	ctx->state = SP_STATE_RESET;

	rc = spm_sp_synchronous_entry(ctx);
	assert(rc == 0);

	ctx->state = SP_STATE_IDLE;

	INFO("Secure Partition initialized.\n");

	return !rc;
}

static int spmc_sp_setup(sp_context_t *ctx)
{
	spm_sp_setup(ctx);

	/* Register init function for deferred init.  */
	bl31_register_bl32_init(&sp_init);

	INFO("Secure Partition setup done.\n");

	return 0;
}

/*******************************************************************************
 * Function to perform a call to a Secure Partition.
 ******************************************************************************/
static uint64_t spmc_sp_call(uint32_t smc_fid, uint64_t comm_buffer_address,
			uint64_t comm_size, uint64_t core_pos)
{
	uint64_t rc;
	sp_context_t *sp_ptr = &sp_ctx;

	/* Set values for registers on SP entry */
	cpu_context_t *cpu_ctx = &(sp_ptr->cpu_ctx);

	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X0, smc_fid);
	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X1, 0);
	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X2, 0);
	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X3, comm_buffer_address);
	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X4, comm_size);
	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X5, 0);
	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X6, core_pos);

	/* Jump to the Secure Partition. */
	rc = spm_sp_synchronous_entry(sp_ptr);

	return rc;
}

/*******************************************************************************
 * Return FFA_ERROR with specified error code
 ******************************************************************************/
static uint64_t spmc_ffa_error_return(void *handle, int error_code)
{
	SMC_RET8(handle, FFA_ERROR,
		 FFA_TARGET_INFO_MBZ, error_code,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ);
}

/*******************************************************************************
 * MM_INTERFACE handler
 ******************************************************************************/
static uint64_t spmc_mm_interface_handler(uint32_t smc_fid, uint64_t mm_cookie,
			       uint64_t comm_buffer_address,
			       uint64_t comm_size_address, void *handle)
{
	uint64_t rc;

	/* Cookie. Reserved for future use. It must be zero. */
	if (mm_cookie != 0U) {
		ERROR("MM_INTERFACE: cookie is not zero\n");
		return spmc_ffa_error_return(handle,
				FFA_ERROR_INVALID_PARAMETER);
	}

	if (comm_buffer_address == 0U) {
		ERROR("MM_INTERFACE: comm_buffer_address is zero\n");
		return spmc_ffa_error_return(handle,
				FFA_ERROR_INVALID_PARAMETER);
	}

	if (comm_size_address != 0U) {
		VERBOSE("MM_INTERFACE: comm_size_address is not 0 as recommended.\n");
	}

	/*
	 * The current secure partition design mandates
	 * - at any point, only a single core can be
	 *   executing in the secure partition.
	 * - a core cannot be preempted by an interrupt
	 *   while executing in secure partition.
	 * Raise the running priority of the core to the
	 * interrupt level configured for secure partition
	 * so as to block any interrupt from preempting this
	 * core.
	 */
	ehf_activate_priority(PLAT_SP_PRI);

	/* Save the Normal world context */
	cm_el1_sysregs_context_save(NON_SECURE);

	rc = spmc_sp_call(smc_fid, comm_buffer_address, comm_size_address, plat_my_core_pos());

	/* Restore non-secure state */
	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	/*
	 * Exited from secure partition. This core can take
	 * interrupts now.
	 */
	ehf_deactivate_priority(PLAT_SP_PRI);

	if (rc == 0)
		SMC_RET2(handle, FFA_SUCCESS_SMC64, 0x7);

	return spmc_ffa_error_return(handle,
			FFA_ERROR_NOT_SUPPORTED);
}

static uint64_t direct_req_secure_smc_handler(uint64_t x1, uint64_t x2,
				uint64_t x3, uint64_t x4, void *cookie,
				void *handle, uint64_t flags)
{
	int32_t rc;

	/* Make next ERET jump to S-EL0 instead of S-EL1. */
	cm_set_elr_spsr_el3(SECURE, read_elr_el1(), read_spsr_el1());

	switch (x3) {
	case SP_MEMORY_ATTRIBUTES_GET_AARCH64:
		INFO("Received SP_MEMORY_ATTRIBUTES_GET_AARCH64 request\n");

		if (sp_ctx.state != SP_STATE_RESET) {
			WARN("SP_MEMORY_ATTRIBUTES_GET_AARCH64 is available at boot time\n");
			return spmc_ffa_error_return(handle,
					FFA_ERROR_NOT_SUPPORTED);
		}

		rc = spm_memory_attributes_get_smc_handler(&sp_ctx, x4);
		if (rc < 0)
			return spmc_ffa_error_return(handle,
					FFA_ERROR_INVALID_PARAMETER);

		SMC_RET4(handle, FFA_MSG_SEND_DIRECT_RESP_SMC64, 0x0, 0x0, rc);

	case SP_MEMORY_ATTRIBUTES_SET_AARCH64:
		INFO("Received SP_MEMORY_ATTRIBUTES_SET_AARCH64 request\n");

		if (sp_ctx.state != SP_STATE_RESET) {
			WARN("SP_MEMORY_ATTRIBUTES_SET_AARCH64 is available at boot time\n");
			return spmc_ffa_error_return(handle,
					FFA_ERROR_NOT_SUPPORTED);
		}

		rc =  spm_memory_attributes_set_smc_handler(&sp_ctx, x4,
				SMC_GET_GP(handle, CTX_GPREG_X5),
				SMC_GET_GP(handle, CTX_GPREG_X6));

		if (rc < 0)
			return spmc_ffa_error_return(handle,
					FFA_ERROR_INVALID_PARAMETER);

		SMC_RET4(handle, FFA_MSG_SEND_DIRECT_RESP_SMC64, 0x0, 0x0, rc);

	default:
		WARN("Not supported direct request handling for ID=0x%llx\n", x3);
		break;
	}

	return spmc_ffa_error_return(handle,
			FFA_ERROR_NOT_SUPPORTED);
}

static uint64_t direct_req_non_secure_smc_handler(uint64_t x1,
			uint64_t x2, uint64_t x3, uint64_t x4,
			void *cookie, void *handle, uint64_t flags)
{

	switch (x3) {
	case MM_INTERFACE_ID_AARCH32:
	case MM_INTERFACE_ID_AARCH64:
		INFO("MM interface id\n");
		return spmc_mm_interface_handler(x3, x4,
				SMC_GET_GP(handle, CTX_GPREG_X5),
				SMC_GET_GP(handle, CTX_GPREG_X6), handle);
	default:
		WARN("Not supported direct request handling for ID=0x%llx\n", x3);
		break;

	}

	return spmc_ffa_error_return(handle,
			FFA_ERROR_NOT_SUPPORTED);
}

static uint64_t direct_req_smc_handler(uint64_t x1,
		uint64_t x2, uint64_t x3, uint64_t x4,
		void *cookie, void *handle, uint64_t flags)
{
	unsigned int ns;

	ns = is_caller_non_secure(flags);
	if (ns == SMC_FROM_SECURE) {
		assert(handle == cm_get_context(SECURE));
		return direct_req_secure_smc_handler(x1, x2, x3, x4, cookie, handle, flags);
	} else {
		assert(handle == cm_get_context(NON_SECURE));
		return direct_req_non_secure_smc_handler(x1, x2, x3, x4, cookie, handle, flags);
	}
}

void spmc_config_addr(uintptr_t soc_fw_config)
{
	spmc_manifest = (void *)soc_fw_config;
}

void *spmc_get_config_addr(void)
{
	return ((void *)spmc_manifest);
}

/*******************************************************************************
 * Initialize contexts of all Secure Partitions.
 ******************************************************************************/
int32_t spmc_setup(void)
{
	sp_context_t *ctx;
	int ret;

	/* Disable MMU at EL1 (initialized by BL2) */
	disable_mmu_icache_el1();

	/* Initialize context of the SP */
	INFO("Secure Partition context setup start...\n");

	ctx = &sp_ctx;

	/* Assign translation tables context. */
	ctx->xlat_ctx_handle = spm_get_sp_xlat_context();

	ret = spmc_sp_setup(ctx);
	if (ret) {
		ERROR(" Error in Secure Partition(SP) setup.\n");
		return ret;
	}

	INFO("Secure Partition Manager(SPMC) setup done.\n");

	return 0;
}

/*******************************************************************************
 * Secure Partition Manager SMC handler.
 ******************************************************************************/
uint64_t spmc_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	switch (smc_fid) {
	case FFA_MSG_SEND_DIRECT_REQ_SMC64:
		return direct_req_smc_handler(x1, x2, x3, x4, cookie, handle, flags);

	case FFA_MSG_SEND_DIRECT_RESP_SMC64:
		spm_sp_synchronous_exit(&sp_ctx, x4);
	default:
		WARN("Not Supported 0x%x FFA Request ID\n", smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}
