/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/*******************************************************************************
 * This is the Secure Payload Dispatcher (SPD). The dispatcher is meant to be a
 * plug-in component to the Secure Monitor, registered as a runtime service. The
 * SPD is expected to be a functional extension of the Secure Payload (SP) that
 * executes in Secure EL1. The Secure Monitor will delegate all SMCs targeting
 * the Trusted OS/Applications range to the dispatcher. The SPD will either
 * handle the request locally or delegate it to the Secure Payload. It is also
 * responsible for initialising and maintaining communication with the SP.
 ******************************************************************************/
#include <arch_helpers.h>
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>
#include <tools_share/uuid.h>

#include "pnc.h"
#include "pncd_private.h"

#define CONFIG_NS_SGI   (PNC_NS_SGI)
#define CONFIG_S_SGI    (15)

/*******************************************************************************
 * Array to keep track of per-cpu Secure Payload state
 ******************************************************************************/
pnc_context_t pncd_sp_context[PNCD_CORE_COUNT];

static unsigned char ree_info = 0;
static uint64_t ree_base_addr;
static uint64_t ree_length;
static uint64_t ree_tag;
static int first = 1;

static int32_t pncd_init(void);

static void context_save(uint32_t security_state)
{
	cm_el1_sysregs_context_save(security_state);
#if CTX_EAGER_SAVE_FPREGS
	fpregs_context_save(get_fpregs_ctx(cm_get_context(security_state)));
#endif
}

static void context_restore(uint32_t security_state)
{
	cm_el1_sysregs_context_restore(security_state);
#if CTX_EAGER_SAVE_FPREGS
	fpregs_context_restore(get_fpregs_ctx(cm_get_context(security_state)));
#endif
}

/*******************************************************************************
 * This function is the handler registered for S-EL1 interrupts by the PNCD. It
 * validates the interrupt and upon success arranges entry into the PNC at
 * 'pnc_sel1_intr_entry()' for handling the interrupt.
 ******************************************************************************/
static uint64_t pncd_sel1_interrupt_handler(uint32_t id,
		uint32_t flags,
		void *handle,
		void *cookie)
{
	/* Check the security state when the exception was generated */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

	/* Sanity check the pointer to this cpu's context */
	assert(handle == cm_get_context(NON_SECURE));

	/* Save the non-secure context before entering the PNC */
	cm_el1_sysregs_context_save(NON_SECURE);

	handle = cm_get_context(SECURE);
	context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	SMC_RET0(handle);
}

/*******************************************************************************
 * Secure Payload Dispatcher setup. The SPD finds out the SP entrypoint and type
 * (aarch32/aarch64) if not already known and initialises the context for entry
 * into the SP for its initialisation.
 ******************************************************************************/
static int32_t pncd_setup(void)
{
	entry_point_info_t *pnc_ep_info;
	uint32_t linear_id;

	linear_id = plat_my_core_pos();

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.  TODO: Add support to
	 * conditionally include the SPD service
	 */
	pnc_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!pnc_ep_info) {
		WARN("No PNC info provided by BL2 boot loader, Booting device"
				" without PNC initialization. SMC`s destined for PNC"
				" will return SMC_UNK\n");
		return 1;
	}

	/*
	 * If there's no valid entry point for SP, we return a non-zero value
	 * signalling failure initializing the service. We bail out without
	 * registering any handlers
	 */
	if (!pnc_ep_info->pc) {
		return 1;
	}

	/*
	 * We could inspect the SP image and determine its execution
	 * state i.e whether AArch32 or AArch64. Assuming it's AArch64
	 * for the time being.
	 */
#ifdef EL1S_32
	pncd_init_pnc_ep_state(pnc_ep_info,
			PNC_AARCH32,
			pnc_ep_info->pc,
			&pncd_sp_context[linear_id]);
#else
	pncd_init_pnc_ep_state(pnc_ep_info,
			PNC_AARCH64,
			pnc_ep_info->pc,
			&pncd_sp_context[linear_id]);
#endif

#if PNC_INIT_ASYNC
	bl31_set_next_image_type(SECURE);
#else
	/*
	 * All PNCD initialization done. Now register our init function with
	 * BL31 for deferred invocation
	 */
	bl31_register_bl32_init(&pncd_init);
	bl31_set_next_image_type(NON_SECURE);
#endif
	return 0;
}

/*******************************************************************************
 * This function passes control to the Secure Payload image (BL32) for the first
 * time on the primary cpu after a cold boot. It assumes that a valid secure
 * context has already been created by pncd_setup() which can be directly used.
 * It also assumes that a valid non-secure context has been initialised by PSCI
 * so it does not need to save and restore any non-secure state. This function
 * performs a synchronous entry into the Secure payload. The SP passes control
 * back to this routine through a SMC.
 ******************************************************************************/
static int32_t pncd_init(void)
{
	uint32_t linear_id = plat_my_core_pos();
	pnc_context_t *pnc_ctx = &pncd_sp_context[linear_id];
	entry_point_info_t *pnc_entry_point;
	uint64_t rc;

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.
	 */
	pnc_entry_point = bl31_plat_get_next_image_ep_info(SECURE);
	assert(pnc_entry_point);

	cm_init_my_context(pnc_entry_point);

	/*
	 * Arrange for an entry into the test secure payload. It will be
	 * returned via PNC_ENTRY_DONE case
	 */
	rc = pncd_synchronous_sp_entry(pnc_ctx);
	/*
	 * We can actually return from pncd_synchronous_sp_entry as
	 * pncd_synchronous_sp_exit will restore the stack when it is called
	 * when we receive the first SMC_YIELD. If everything went well at this
	 * point, the return value should be 0.
	 */
	/* NOTE: bl31_main() may yield a warning if rc = 0 */

	return rc;
}

uint32_t rpmb_keys[32];

static int rpmb_key_get(uint32_t *key, uint32_t offset)
{
	if (offset > offset + 4 ||
			offset + 4 > ARRAY_SIZE(rpmb_keys)) {
		INFO("%s: requested offset is too big (%u)\n", __func__, offset);
		return -EINVAL;
	}

	key[0] = rpmb_keys[offset];
	key[1] = rpmb_keys[offset + 1];
	key[2] = rpmb_keys[offset + 2];
	key[3] = rpmb_keys[offset + 3];
	rpmb_keys[offset] = 0;
	rpmb_keys[offset + 1] = 0;
	rpmb_keys[offset + 2] = 0;
	rpmb_keys[offset + 3] = 0;

	return 0;
}

/*******************************************************************************
 * This function is responsible for handling all SMCs in the Trusted OS/App
 * range from the non-secure state as defined in the SMC Calling Convention
 * Document. It is also responsible for communicating with the Secure payload
 * to delegate work and return results back to the non-secure state. Lastly it
 * will also return any information that the secure payload needs to do the
 * work assigned to it.
 ******************************************************************************/
static uintptr_t pncd_smc_handler(uint32_t smc_fid,
		u_register_t x1,
		u_register_t x2,
		u_register_t x3,
		u_register_t x4,
		void *cookie,
		void *handle,
		u_register_t flags)
{
	uint32_t linear_id = plat_my_core_pos(), ns;
	pnc_context_t *pnc_ctx = &pncd_sp_context[linear_id];
	uint64_t rc;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	switch (smc_fid) {
	case SMC_GET_SHAREDMEM:
		if (ree_info == 1) {
			x1 = (((uint32_t)ree_info) << 16) | ree_tag;
			x2 = ree_base_addr & 0xFFFFFFFF;
			x3 = (ree_base_addr >> 32) & 0xFFFFFFFF;
			x4 = ree_length & 0xFFFFFFFF;
			SMC_RET4(handle, x1, x2, x3, x4);
		} else {
			SMC_RET4(handle, 0, 0, 0, 0);
		}
	case SMC_CONFIG_SHAREDMEM:
		if (ree_info == 1) {
			/* Do not Yield */
			SMC_RET0(handle);
		}

		// a1 is physical base address, a2 is size in bytes
		ree_base_addr = x1 | (x2 << 32);
		ree_length = x3;
		ree_tag = x4;
		INFO("IN SMC_CONFIG_SHAREDMEM: addr=%lx, length=%lx, tag =%lx\n",
						(long unsigned int) ree_base_addr,
						(long unsigned int) ree_length,
						(long unsigned int) ree_tag);
		if ((ree_base_addr % 0x200000) != 0) {
			SMC_RET1(handle, SMC_UNK);
		}

		if ((ree_length % 0x200000) != 0) {
			SMC_RET1(handle, SMC_UNK);
		}
		ree_info = 1;
		/* Do not Yield */
		SMC_RET4(handle, 0, 0, 0, 0);

	case SMC_ACTION_FROM_NS:
		if (!ns) {
			SMC_RET1(handle, SMC_UNK);
		}

		if (CONFIG_S_SGI < MIN_PPI_ID) {
			plat_ic_raise_s_el1_sgi(CONFIG_S_SGI, read_mpidr());
		} else {
			plat_ic_set_interrupt_pending(CONFIG_S_SGI);
		}

		SMC_RET0(handle);

	case SMC_ACTION_FROM_S:
		if (ns) {
			SMC_RET1(handle, SMC_UNK);
		}

		if (CONFIG_NS_SGI < MIN_PPI_ID) {
			plat_ic_raise_ns_sgi(CONFIG_NS_SGI, read_mpidr());
		} else {
			plat_ic_set_interrupt_pending(CONFIG_NS_SGI);
		}
		SMC_RET0(handle);

	case SMC_YIELD:
		if (first) {
			first = 0;
			INFO("IN PNC_ENTRY_DONE\n");
			handle = cm_get_context(SECURE);
			context_save(SECURE);

			set_pnc_pstate(pnc_ctx->state, PNC_PSTATE_ON);

			/*
			 * PNC has been successfully initialized. Register power
			 * managemnt hooks with PSCI
			 */
			psci_register_spd_pm_hook(&pncd_pm);
			/*
			 * Register an interrupt handler for S-EL1 interrupts
			 * when generated during code executing in the
			 * non-secure state.
			 */
			flags = 0;
			set_interrupt_rm_flag(flags, NON_SECURE);
			rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
					pncd_sel1_interrupt_handler,
					flags);
			if (rc) {
				panic();
			}

			/*
			 * SP reports completion. The SPD must have initiated
			 * the original request through a synchronous entry
			 * into the SP. Jump back to the original C runtime
			 * context.
			 */
			pncd_synchronous_sp_exit(pnc_ctx, (uint64_t) 0x0);
		} else {
			if (ns) {
				assert(handle == cm_get_context(NON_SECURE));
				context_save(NON_SECURE);
				/* Get a reference to the non-secure context */
				handle = cm_get_context(SECURE);
				assert(handle);

				/* Restore non-secure state */
				context_restore(SECURE);
				cm_set_next_eret_context(SECURE);
				SMC_RET0(handle);
			} else {
				assert(handle == cm_get_context(SECURE));
				context_save(SECURE);
				/* Get a reference to the non-secure context */
				handle = cm_get_context(NON_SECURE);
				assert(handle);

				/* Restore non-secure state */
				context_restore(NON_SECURE);
				cm_set_next_eret_context(NON_SECURE);
				SMC_RET0(handle);
			}
		}

	case SMC_GET_RPMB_KEY:
		{
			uint32_t key[4];
			rc = rpmb_key_get(key, x1);
			if (rc) {
				SMC_RET0(handle);
			}
			SMC_RET4(handle, key[0], key[1], key[2], key[3]);
		}

	default:
		INFO("Unknown smc: %x\n", smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}

/* Define a SPD runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	pncd_fast,
	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_FAST,
	pncd_setup,
	pncd_smc_handler
);

/* Define a SPD runtime service descriptor for standard SMC calls */
DECLARE_RT_SVC(
	pncd_std,
	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_YIELD,
	NULL,
	pncd_smc_handler
);
