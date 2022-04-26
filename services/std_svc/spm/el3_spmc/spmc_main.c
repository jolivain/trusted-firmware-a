/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <bl31/ehf.h>
#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/smccc.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <libfdt.h>
#include "logical_sp.h"
#include <plat/common/platform.h>
#include <services/ffa_svc.h>
#include <services/spmc_svc.h>
#include <services/spmd_svc.h>
#include "spmc.h"
#include "spmc_shared_mem.h"

#include <platform_def.h>

/*
 * Allocate a secure partition descriptor to describe each SP in the system that
 * does not reside at EL3.
 */
static struct secure_partition_desc sp_desc[SECURE_PARTITION_COUNT];

/*
 * Allocate an NS endpoint descriptor to describe each VM and the Hypervisor in
 * the system that interacts with a SP. It is used to track the Hypervisor
 * buffer pair, version and ID for now. It could be extended to track VM
 * properties when the SPMC supports indirect messaging.
 */
static struct ns_endpoint_desc ns_ep_desc[NS_PARTITION_COUNT];

static uint64_t spmc_sp_interrupt_handler(uint32_t id,
					  uint32_t flags,
					  void *handle,
					  void *cookie);


/*
 * Helper function to obtain the array storing the EL3
 * Logical Partition descriptors.
 */
struct el3_lp_desc *get_el3_lp_array(void)
{
	return (struct el3_lp_desc *) EL3_LP_DESCS_START;
}

/*
 * Helper function to obtain the descriptor of the last SP to whom control was
 * handed to on this physical cpu. Currently, we assume there is only one SP.
 * TODO: Expand to track multiple partitions when required.
 */
struct secure_partition_desc *spmc_get_current_sp_ctx(void)
{
	return &(sp_desc[ACTIVE_SP_DESC_INDEX]);
}

/*
 * Helper function to obtain the execution context of an SP on the
 * current physical cpu.
 */
struct sp_exec_ctx *spmc_get_sp_ec(struct secure_partition_desc *sp)
{
	return &(sp->ec[get_ec_index(sp)]);
}

/* Helper function to get pointer to SP context from its ID. */
struct secure_partition_desc *spmc_get_sp_ctx(uint16_t id)
{
	/* Check for SWd Partitions. */
	for (unsigned int i = 0U; i < SECURE_PARTITION_COUNT; i++) {
		if (sp_desc[i].sp_id == id) {
			return &(sp_desc[i]);
		}
	}
	return NULL;
}

/*
 * Helper function to obtain the descriptor of the Hypervisor or OS kernel.
 * We assume that the first descriptor is reserved for this entity.
 */
struct ns_endpoint_desc *spmc_get_hyp_ctx(void)
{
	return &(ns_ep_desc[0]);
}

/*
 * Helper function to obtain the RX/TX buffer pair descriptor of the Hypervisor
 * or OS kernel in the normal world or the last SP that was run.
 */
struct mailbox *spmc_get_mbox_desc(bool secure_origin)
{
	/* Obtain the RX/TX buffer pair descriptor. */
	if (secure_origin) {
		return &(spmc_get_current_sp_ctx()->mailbox);
	} else {
		return &(spmc_get_hyp_ctx()->mailbox);
	}
}

/******************************************************************************
 * This function returns to the place where spmc_sp_synchronous_entry() was
 * called originally.
 ******************************************************************************/
__dead2 void spmc_sp_synchronous_exit(struct sp_exec_ctx *ec, uint64_t rc)
{
	/*
	 * The SPM must have initiated the original request through a
	 * synchronous entry into the secure partition. Jump back to the
	 * original C runtime context with the value of rc in x0;
	 */
	spm_secure_partition_exit(ec->c_rt_ctx, rc);

	panic();
}

/*******************************************************************************
 * Return FFA_ERROR with specified error code.
 ******************************************************************************/
uint64_t spmc_ffa_error_return(void *handle, int error_code)
{
	SMC_RET8(handle, FFA_ERROR,
		 FFA_TARGET_INFO_MBZ, error_code,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ);
}

/******************************************************************************
 * Helper function to validate a secure partition ID to ensure it does not
 * conflict with any other FF-A component and follows the convention to
 * indicate it resides within the secure world.
 ******************************************************************************/
bool is_ffa_secure_id_valid(uint16_t partition_id)
{
	struct el3_lp_desc *el3_lp_descs = get_el3_lp_array();

	/* Ensure the ID is not the invalid partition ID. */
	if (partition_id == INV_SP_ID) {
		return false;
	}

	/* Ensure the ID is not the SPMD ID. */
	if (partition_id == SPMD_DIRECT_MSG_ENDPOINT_ID) {
		return false;
	}

	/*
	 * Ensure the ID follows the convention to indicate it resides
	 * in the secure world.
	 */
	if (!ffa_is_secure_world_id(partition_id)) {
		return false;
	}

	/* Ensure we don't conflict with the SPMC partition ID. */
	if (partition_id == FFA_SPMC_ID) {
		return false;
	}

	/* Ensure we do not already have an SP context with this ID. */
	if (spmc_get_sp_ctx(partition_id)) {
		return false;
	}

	/* Ensure we don't clash with any Logical SP's. */
	for (unsigned int i = 0U; i < EL3_LP_DESCS_COUNT; i++) {
		if (el3_lp_descs[i].sp_id == partition_id) {
			return false;
		}
	}

	return true;
}

/*******************************************************************************
 * This function either forwards the request to the other world or returns
 * with an ERET depending on the source of the call.
 * We can assume that the destination is for an entity at a lower exception
 * level as any messages destined for a logical SP resident in EL3 will have
 * already been taken care of by the SPMC before entering this function.
 ******************************************************************************/
static uint64_t spmc_smc_return(uint32_t smc_fid,
				bool secure_origin,
				uint64_t x1,
				uint64_t x2,
				uint64_t x3,
				uint64_t x4,
				void *handle,
				void *cookie,
				uint64_t flags,
				uint16_t dst_id)
{
	/* If the destination is in the normal world always go via the SPMD. */
	if (ffa_is_normal_world_id(dst_id)) {
		return spmd_smc_handler(smc_fid, x1, x2, x3, x4,
					cookie, handle, flags);
	}
	/*
	 * If the caller is secure and we want to return to the secure world,
	 * ERET directly.
	 */
	else if (secure_origin && ffa_is_secure_world_id(dst_id)) {
		SMC_RET5(handle, smc_fid, x1, x2, x3, x4);
	}
	/* If we originated in the normal world then switch contexts. */
	else if (!secure_origin && ffa_is_secure_world_id(dst_id)) {
		return spmd_smc_switch_state(smc_fid, secure_origin, x1, x2,
					     x3, x4, handle);
	} else {
		/* Unknown State. */
		panic();
	}

	/* Shouldn't be Reached. */
	return 0;
}

/*******************************************************************************
 * FF-A ABI Handlers.
 ******************************************************************************/

/*******************************************************************************
 * Helper function to validate arg2 as part of a direct message.
 ******************************************************************************/
static inline bool direct_msg_validate_arg2(uint64_t x2)
{
	/* Check message type. */
	if (x2 & FFA_DIRECT_FRAMEWORK_MSG_MASK) {
		/* We have a framework message, ensure it is a PM message. */
		if (x2 & (~FFA_PM_MSG_MASK &
			  ~(FFA_DIRECT_FRAMEWORK_MSG_MASK))) {
			VERBOSE("Unknown Framework message 0x%lx.\n", x2);
			return false;
		}
	} else {
		/* We have a partition messages, ensure x2 is not set. */
		if (x2 != (uint64_t) 0) {
			VERBOSE("Arg2 MBZ for partition messages. (0x%lx).\n",
				x2);
			return false;
		}
	}
	return true;
}

/*******************************************************************************
 * Handle direct request messages and route to the appropriate destination.
 ******************************************************************************/
static uint64_t direct_req_smc_handler(uint32_t smc_fid,
				       bool secure_origin,
				       uint64_t x1,
				       uint64_t x2,
				       uint64_t x3,
				       uint64_t x4,
				       void *cookie,
				       void *handle,
				       uint64_t flags)
{
	uint16_t dst_id = ffa_endpoint_destination(x1);
	struct el3_lp_desc *el3_lp_descs;
	struct secure_partition_desc *sp;
	unsigned int idx;

	/* Check if arg2 has been populated correctly based on message type. */
	if (!direct_msg_validate_arg2(x2)) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	el3_lp_descs = get_el3_lp_array();

	/* Check if the request is destined for a Logical Partition. */
	for (unsigned int i = 0U; i < MAX_EL3_LP_DESCS_COUNT; i++) {
		if (el3_lp_descs[i].sp_id == dst_id) {
			return el3_lp_descs[i].direct_req(
					smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);
		}
	}

	/*
	 * If the request was not targeted to a LSP and from the secure world
	 * then it is invalid since a SP cannot call into the Normal world and
	 * there is no other SP to call into. If there are other SPs in future
	 * then the partition runtime model would need to be validated as well.
	 */
	if (secure_origin) {
		VERBOSE("Direct request not supported to the Normal World.\n");
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Check if the SP ID is valid. */
	sp = spmc_get_sp_ctx(dst_id);
	if (sp == NULL) {
		VERBOSE("Direct request to unknown partition ID (0x%x).\n",
			dst_id);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Check that the target execution context is in a waiting state before
	 * forwarding the direct request to it.
	 */
	idx = get_ec_index(sp);
	if (sp->ec[idx].rt_state != RT_STATE_WAITING) {
		VERBOSE("SP context on core%u is not waiting (%u).\n",
			idx, sp->ec[idx].rt_model);
		return spmc_ffa_error_return(handle, FFA_ERROR_BUSY);
	}

	/*
	 * Everything checks out so forward the request to the SP after updating
	 * its state and runtime model.
	 */
	sp->ec[idx].rt_state = RT_STATE_RUNNING;
	sp->ec[idx].rt_model = RT_MODEL_DIR_REQ;
	return spmc_smc_return(smc_fid, secure_origin, x1, x2, x3, x4,
			       handle, cookie, flags, dst_id);
}

/*******************************************************************************
 * Handle direct response messages and route to the appropriate destination.
 ******************************************************************************/
static uint64_t direct_resp_smc_handler(uint32_t smc_fid,
					bool secure_origin,
					uint64_t x1,
					uint64_t x2,
					uint64_t x3,
					uint64_t x4,
					void *cookie,
					void *handle,
					uint64_t flags)
{
	uint16_t dst_id = ffa_endpoint_destination(x1);
	struct secure_partition_desc *sp;
	unsigned int idx;

	/* Check if arg2 has been populated correctly based on message type. */
	if (!direct_msg_validate_arg2(x2)) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Check that the response did not originate from the Normal world. */
	if (!secure_origin) {
		VERBOSE("Direct Response not supported from Normal World.\n");
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Check that the response is either targeted to the Normal world or the
	 * SPMC e.g. a PM response.
	 */
	if ((dst_id != FFA_SPMC_ID) && ffa_is_secure_world_id(dst_id)) {
		VERBOSE("Direct response to invalid partition ID (0x%x).\n",
			dst_id);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Obtain the SP descriptor and update its runtime state. */
	sp = spmc_get_sp_ctx(ffa_endpoint_source(x1));
	if (sp == NULL) {
		VERBOSE("Direct response to unknown partition ID (0x%x).\n",
			dst_id);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Sanity check state is being tracked correctly in the SPMC. */
	idx = get_ec_index(sp);
	assert(sp->ec[idx].rt_state == RT_STATE_RUNNING);

	/* Ensure SP execution context was in the right runtime model. */
	if (sp->ec[idx].rt_model != RT_MODEL_DIR_REQ) {
		VERBOSE("SP context on core%u not handling direct req (%u).\n",
			idx, sp->ec[idx].rt_model);
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Update the state of the SP execution context. */
	sp->ec[idx].rt_state = RT_STATE_WAITING;

	/*
	 * If the receiver is not the SPMC then forward the response to the
	 * Normal world.
	 */
	if (dst_id == FFA_SPMC_ID) {
		spmc_sp_synchronous_exit(&sp->ec[idx], x4);
		/* Should not get here. */
		panic();
	}

	return spmc_smc_return(smc_fid, secure_origin, x1, x2, x3, x4,
			       handle, cookie, flags, dst_id);
}

/*******************************************************************************
 * This function handles the FFA_MSG_WAIT SMC to allow an SP to relinquish its
 * cycles.
 ******************************************************************************/
static uint64_t msg_wait_handler(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *cookie,
				 void *handle,
				 uint64_t flags)
{
	struct secure_partition_desc *sp;
	unsigned int idx;

	/*
	 * Check that the response did not originate from the Normal world as
	 * only the secure world can call this ABI.
	 */
	if (!secure_origin) {
		VERBOSE("Normal world cannot call FFA_MSG_WAIT.\n");
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	/* Get the descriptor of the SP that invoked FFA_MSG_WAIT. */
	sp = spmc_get_current_sp_ctx();
	if (sp == NULL) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Get the execution context of the SP that invoked FFA_MSG_WAIT.
	 */
	idx = get_ec_index(sp);

	/* Ensure SP execution context was in the right runtime model. */
	if (sp->ec[idx].rt_model == RT_MODEL_DIR_REQ) {
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Sanity check the state is being tracked correctly in the SPMC. */
	assert(sp->ec[idx].rt_state == RT_STATE_RUNNING);

	/*
	 * Perform a synchronous exit if the partition was initialising. The
	 * state is updated after the exit.
	 */
	if (sp->ec[idx].rt_model == RT_MODEL_INIT) {
		spmc_sp_synchronous_exit(&sp->ec[idx], x4);
		/* Should not get here */
		panic();
	}

	/* Update the state of the SP execution context. */
	sp->ec[idx].rt_state = RT_STATE_WAITING;

	/* Resume normal world if a secure interrupt was handled. */
	if (sp->ec[idx].rt_model == RT_MODEL_INTR) {
		/* FFA_MSG_WAIT can only be called from the secure world. */
		unsigned int secure_state_in = SECURE;
		unsigned int secure_state_out = NON_SECURE;

		cm_el1_sysregs_context_save(secure_state_in);
		cm_el1_sysregs_context_restore(secure_state_out);
		cm_set_next_eret_context(secure_state_out);
		SMC_RET0(cm_get_context(secure_state_out));
	}

	/* Forward the response to the Normal world. */
	return spmc_smc_return(smc_fid, secure_origin, x1, x2, x3, x4,
			       handle, cookie, flags, FFA_NWD_ID);
}

static uint64_t ffa_error_handler(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *cookie,
				 void *handle,
				 uint64_t flags)
{
	struct secure_partition_desc *sp;
	unsigned int idx;

	/* Check that the response did not originate from the Normal world. */
	if (!secure_origin) {
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	/* Get the descriptor of the SP that invoked FFA_ERROR. */
	sp = spmc_get_current_sp_ctx();
	if (sp == NULL) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Get the execution context of the SP that invoked FFA_ERROR. */
	idx = get_ec_index(sp);

	/*
	 * We only expect FFA_ERROR to be received during SP initialisation
	 * otherwise this is an invalid call.
	 */
	if (sp->ec[idx].rt_model == RT_MODEL_INIT) {
		ERROR("SP 0x%x failed to initialize.\n", sp->sp_id);
		spmc_sp_synchronous_exit(&sp->ec[idx], x2);
		/* Should not get here. */
		panic();
	}

	return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
}

static uint64_t ffa_version_handler(uint32_t smc_fid,
				    bool secure_origin,
				    uint64_t x1,
				    uint64_t x2,
				    uint64_t x3,
				    uint64_t x4,
				    void *cookie,
				    void *handle,
				    uint64_t flags)
{
	uint32_t requested_version = x1;

	if (requested_version & FFA_VERSION_BIT31_MASK) {
		/* Invalid encoding, return an error. */
		SMC_RET1(handle, FFA_ERROR_NOT_SUPPORTED);
		/* Execution stops here. */
	}

	/* Check if a supported FF-A version. */
	if (requested_version != MAKE_FFA_VERSION(U(1), U(0)) &&
	    requested_version != MAKE_FFA_VERSION(U(1), U(1))) {
		SMC_RET1(handle, FFA_ERROR_NOT_SUPPORTED);
		/* Execution stops here. */
	}

	/* Determine the caller to store the requested version. */
	if (secure_origin) {
		/*
		 * Ensure that the SP is reporting the same version as
		 * specified in it's manifest. If these do not match there is
		 * something wrong with the SP.
		 * TODO: Should we abort the SP? For now assert this is not
		 *       case.
		 */
		assert(requested_version ==
		       spmc_get_current_sp_ctx()->ffa_version);
	} else {
		/*
		 * If this is called by the normal world, record this
		 * information in it's descriptor.
		 */
		spmc_get_hyp_ctx()->ffa_version = requested_version;
	}

	SMC_RET1(handle, MAKE_FFA_VERSION(FFA_VERSION_MAJOR,
					  FFA_VERSION_MINOR));
}

/*
 * Add helper function to obtain the FF-A version of the calling
 * partition.
 */
uint32_t get_partition_ffa_version(bool secure_origin)
{
	if (secure_origin) {
		return spmc_get_current_sp_ctx()->ffa_version;
	} else {
		return spmc_get_hyp_ctx()->ffa_version;
	}
}

static uint64_t rxtx_map_handler(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *cookie,
				 void *handle,
				 uint64_t flags)
{
	int ret;
	uint32_t error_code;
	uint32_t mem_atts = secure_origin ? MT_SECURE : MT_NS;
	struct mailbox *mbox;
	uintptr_t tx_address = x1;
	uintptr_t rx_address = x2;
	uint32_t page_count = x3 & FFA_RXTX_PAGE_COUNT_MASK; /* Bits [5:0] */
	uint32_t buf_size = page_count * FFA_PAGE_SIZE;

	/*
	 * The SPMC does not support mapping of VM RX/TX pairs to facilitate
	 * indirect messaging with SPs. Check if the Hypervisor has invoked this
	 * ABI on behalf of a VM and reject it if this is the case.
	 */
	if (tx_address == 0 || rx_address == 0) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Ensure the specified buffers are not the same. */
	if (tx_address == rx_address) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Ensure the buffer size is a multiple of the translation granule size
	 * in TF-A.
	 */
	if (buf_size % PAGE_SIZE != 0U) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Obtain the RX/TX buffer pair descriptor. */
	mbox = spmc_get_mbox_desc(secure_origin);

	spin_lock(&mbox->lock);

	/* Check if buffers have already been mapped. */
	if (mbox->rx_buffer != 0 || mbox->tx_buffer != 0) {
		WARN("RX/TX Buffers already mapped (%p/%p)\n",
		     (void *) mbox->rx_buffer, (void *)mbox->tx_buffer);
		error_code = FFA_ERROR_DENIED;
		goto err;
	}

	/* memmap the TX buffer as read only. */
	ret = mmap_add_dynamic_region(tx_address, /* PA */
			tx_address, /* VA */
			buf_size, /* size */
			mem_atts | MT_RO_DATA); /* attrs */
	if (ret != 0) {
		/* Return the correct error code. */
		error_code = (ret == -ENOMEM) ? FFA_ERROR_NO_MEMORY :
						FFA_ERROR_INVALID_PARAMETER;
		WARN("Unable to map TX buffer: %d\n", error_code);
		goto err;
	}

	/* memmap the RX buffer as read write. */
	ret = mmap_add_dynamic_region(rx_address, /* PA */
			rx_address, /* VA */
			buf_size, /* size */
			mem_atts | MT_RW_DATA); /* attrs */

	if (ret != 0) {
		error_code = (ret == -ENOMEM) ? FFA_ERROR_NO_MEMORY :
						FFA_ERROR_INVALID_PARAMETER;
		WARN("Unable to map RX buffer: %d\n", error_code);
		/* Unmap the TX buffer again. */
		mmap_remove_dynamic_region(tx_address, buf_size);
		goto err;
	}

	mbox->tx_buffer = (void *) tx_address;
	mbox->rx_buffer = (void *) rx_address;
	mbox->rxtx_page_count = page_count;
	spin_unlock(&mbox->lock);

	SMC_RET1(handle, FFA_SUCCESS_SMC32);
	/* Execution stops here. */
err:
	spin_unlock(&mbox->lock);
	return spmc_ffa_error_return(handle, error_code);
}

static uint64_t rxtx_unmap_handler(uint32_t smc_fid,
				   bool secure_origin,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	struct mailbox *mbox = spmc_get_mbox_desc(flags);
	uint32_t buf_size = mbox->rxtx_page_count * FFA_PAGE_SIZE;

	/*
	 * The SPMC does not support mapping of VM RX/TX pairs to facilitate
	 * indirect messaging with SPs. Check if the Hypervisor has invoked this
	 * ABI on behalf of a VM and reject it if this is the case.
	 */
	if (x1 != 0) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	spin_lock(&mbox->lock);

	/* Check if buffers are currently mapped. */
	if (mbox->rx_buffer == 0 || mbox->tx_buffer == 0) {
		spin_unlock(&mbox->lock);
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Unmap RX Buffer */
	if (mmap_remove_dynamic_region((uintptr_t) mbox->rx_buffer,
				       buf_size) != 0) {
		WARN("Unable to unmap RX buffer!\n");
	}

	mbox->rx_buffer = 0;

	/* Unmap TX Buffer */
	if (mmap_remove_dynamic_region((uintptr_t) mbox->tx_buffer,
				       buf_size) != 0) {
		WARN("Unable to unmap TX buffer!\n");
	}

	mbox->tx_buffer = 0;
	mbox->rxtx_page_count = 0;

	spin_unlock(&mbox->lock);
	SMC_RET1(handle, FFA_SUCCESS_SMC32);
}

/*
 * Helper function to check if 2 UUIDs match.
 */
bool uuid_match(uint32_t *uuid1, uint32_t *uuid2)
{
	return !memcmp(uuid1, uuid2, sizeof(uint32_t) * 4);
}

/*
 * Helper function to copy from one UUID struct to another.
 */
void copy_uuid(uint32_t *to_uuid, uint32_t *from_uuid)
{
	to_uuid[0] = from_uuid[0];
	to_uuid[1] = from_uuid[1];
	to_uuid[2] = from_uuid[2];
	to_uuid[3] = from_uuid[3];
}

/*
 * Handle populating the required partition information for a v1.1
 * caller in the appropriate version of the partition information
 * descriptor.
 */
static int partition_info_get_handler_v1_1(uint32_t *uuid,
					   bool is_null_uuid,
					   struct mailbox *mbox,
					   struct el3_lp_desc *el3_lp_descs,
					   int *partition_count)
{
	int index;
	int max_partitions = 0;
	struct ffa_partition_info_v1_1 *partitions;
	struct ffa_partition_info_v1_1 *desc;

	/*
	 * If we are populating the descriptor cast the RX buffer to a v1.1
	 * format, we have already checked that is it not NULL.
	 */
	partitions = (struct ffa_partition_info_v1_1 *) mbox->rx_buffer;

	/* Calculate the maximum number of partitions we can report on. */
	max_partitions = (mbox->rxtx_page_count * FFA_PAGE_SIZE) /
			 sizeof(struct ffa_partition_info_v1_0);

	/* Deal with Logical Partitions. */
	for (index = 0; index < EL3_LP_DESCS_COUNT; index++) {
		if (is_null_uuid ||
		    uuid_match(uuid, el3_lp_descs[index].uuid)) {
			/* Found a matching UUID, populate appropriately. */
			if (*partition_count >= max_partitions) {
				return FFA_ERROR_NO_MEMORY;
			}

			desc = &partitions[*partition_count];
			desc->ep_id = el3_lp_descs[index].sp_id;
			desc->execution_ctx_count = PLATFORM_CORE_COUNT;
			desc->properties = el3_lp_descs[index].properties;
			if (is_null_uuid) {
				copy_uuid(desc->uuid, el3_lp_descs[index].uuid);
			}
			(*partition_count)++;
		}
	}

	/* Deal with physical SP's. */
	for (index = 0; index < SECURE_PARTITION_COUNT; index++) {
		if (is_null_uuid || uuid_match(uuid, sp_desc[index].uuid)) {
			/* Found a matching UUID, populate appropriately. */
			if (*partition_count >= max_partitions) {
				return FFA_ERROR_NO_MEMORY;
			}

			desc = &partitions[*partition_count];
			desc->ep_id = sp_desc[index].sp_id;
			/*
			 * Execution context count must match No. cores for
			 * S-EL1 SPs.
			 */
			desc->execution_ctx_count = PLATFORM_CORE_COUNT;
			desc->properties = sp_desc[index].properties;
			if (is_null_uuid) {
				copy_uuid(desc->uuid, el3_lp_descs[index].uuid);
			}
			(*partition_count)++;
		}
	}
	return 0;
}

/*
 * Handle populating the required partition information for a v1.0
 * caller in the appropriate version of the partition information
 * descriptor.
 */
static int partition_info_get_handler_v1_0(uint32_t *uuid,
					   bool is_null_uuid,
					   struct mailbox *mbox,
					   struct el3_lp_desc *el3_lp_descs,
					   int *partition_count)
{
	int index;
	int max_partitions = 0;
	struct ffa_partition_info_v1_0 *partitions;
	struct ffa_partition_info_v1_0 *desc;

	/*
	 * If we are populating the descriptor cast the RX buffer to a v1.0
	 * format, we have already checked that is it not NULL.
	 */
	partitions = (struct ffa_partition_info_v1_0 *) mbox->rx_buffer;

	/* Calculate the maximum number of partitions we can report on. */
	max_partitions = (mbox->rxtx_page_count * FFA_PAGE_SIZE) /
			 sizeof(struct ffa_partition_info_v1_0);

	/* Deal with Logical Partitions. */
	for (index = 0; index < EL3_LP_DESCS_COUNT; index++) {
		if (is_null_uuid ||
		    uuid_match(uuid, el3_lp_descs[index].uuid)) {
			/* Found a matching UUID, populate appropriately. */
			if (*partition_count >= max_partitions) {
				return FFA_ERROR_NO_MEMORY;
			}

			desc = &partitions[*partition_count];

			desc->ep_id = el3_lp_descs[index].sp_id;
			desc->execution_ctx_count = PLATFORM_CORE_COUNT;
			desc->properties = el3_lp_descs[index].properties;
			(*partition_count)++;
			if (*partition_count > max_partitions) {
				return FFA_ERROR_NO_MEMORY;
			}
		}
	}

	/* Deal with physical SP's. */
	for (index = 0; index < SECURE_PARTITION_COUNT; index++) {
		if (is_null_uuid || uuid_match(uuid, sp_desc[index].uuid)) {
			/* Found a matching UUID, populate appropriately. */
			if (*partition_count >= max_partitions) {
				return FFA_ERROR_NO_MEMORY;
			}

			desc = &partitions[*partition_count];
			desc->ep_id = sp_desc[index].sp_id;
			/*
			 * Execution context count must match No. cores for
			 * S-EL1 SPs.
			 */
			desc->execution_ctx_count = PLATFORM_CORE_COUNT;
			desc->properties = sp_desc[index].properties;
			(*partition_count)++;
		}
	}
	return 0;
}

/*
 * Handle the case where that caller only wants the count of partitions
 * matching a given UUID and does not want the corresponding descriptors
 * populated.
 */
static int partition_info_get_handler_count_only(uint32_t *uuid,
						 bool is_null_uuid,
						 struct el3_lp_desc *el3_lp_descs,
						 int *partition_count)
{
	int index = 0;

	/* Deal with Logical Partitions. */
	for (index = 0; index < EL3_LP_DESCS_COUNT; index++) {
		if (is_null_uuid ||
		    uuid_match(uuid, el3_lp_descs[index].uuid)) {
			(*partition_count)++;
		}
	}

	/* Deal with physical SP's. */
	for (index = 0; index < SECURE_PARTITION_COUNT; index++) {
		if (is_null_uuid || uuid_match(uuid, sp_desc[index].uuid)) {
			(*partition_count)++;
		}
	}

	return 0;
}

/*
 * Main handler for FFA_PARTITION_INFO_GET which supports both FF-A v1.1 and
 * v1.0 implementations. The version of the caller is first determined and the
 * appropriate sub-handler is called depending on the request.
 */
static uint64_t partition_info_get_handler(uint32_t smc_fid,
					   bool secure_origin,
					   uint64_t x1,
					   uint64_t x2,
					   uint64_t x3,
					   uint64_t x4,
					   void *cookie,
					   void *handle,
					   uint64_t flags)
{
	int ret;
	int partition_count = 0;
	uint32_t ffa_version = get_partition_ffa_version(secure_origin);
	struct el3_lp_desc *el3_lp_descs = get_el3_lp_array();
	struct mailbox *mbox;
	uint64_t info_get_flags;
	bool is_null_uuid;
	bool count_only;
	uint32_t uuid[4];

	uuid[0] = x1;
	uuid[1] = x2;
	uuid[2] = x3;
	uuid[3] = x4;

	is_null_uuid = (uuid[0] == 0 && uuid[1] == 0 &&
			uuid[2] == 0 && uuid[3] == 0);

	/* Determine if the Partition descriptors should be populated. */
	info_get_flags = SMC_GET_GP(handle, CTX_GPREG_X5);
	count_only = (info_get_flags & FFA_PARTITION_INFO_GET_COUNT_FLAG_MASK);

	/* Handle the case where we don't need to populate the descriptors. */
	if (count_only) {
		ret = partition_info_get_handler_count_only(uuid,
							    is_null_uuid,
							    el3_lp_descs,
							    &partition_count);

		if (partition_count == 0) {
			return spmc_ffa_error_return(handle,
						FFA_ERROR_INVALID_PARAMETER);
		}
	} else {
		/*
		 * Handle the case where the partition descriptors are required,
		 * check we have the buffers available and populate the
		 * appropriate structure version.
		 */

		/* Obtain the partition mailbox RX/TX buffer pair descriptor. */
		mbox = spmc_get_mbox_desc(secure_origin);

		/*
		 * If the caller has not bothered registering its RX/TX pair
		 * then return an error code.
		 */
		spin_lock(&mbox->lock);
		if (mbox->rx_buffer == 0) {
			ret = FFA_ERROR_BUSY;
			goto err_unlock;
		}

		/* Ensure the RX buffer is currently free. */
		if (mbox->state != MAILBOX_STATE_EMPTY) {
			ret = FFA_ERROR_BUSY;
			goto err_unlock;
		}

		/* Zero the RX buffer before populating. */
		(void)memset(mbox->rx_buffer, 0,
			     mbox->rxtx_page_count * FFA_PAGE_SIZE);

		/*
		 * Depending on the FF-A version of the requesting partition
		 * choose the appropriate handler to populate the descriptor.
		 */
		if (ffa_version == MAKE_FFA_VERSION(U(1), U(0))) {
			ret = partition_info_get_handler_v1_0(uuid,
							      is_null_uuid,
							      mbox,
							      el3_lp_descs,
							      &partition_count);
		} else if (ffa_version == MAKE_FFA_VERSION(U(1), U(1))) {
			ret = partition_info_get_handler_v1_1(uuid,
							      is_null_uuid,
							      mbox,
							      el3_lp_descs,
							      &partition_count);
		} else {
			ERROR("Unknown FF-A version %x\n", ffa_version);
			ret = FFA_ERROR_INVALID_PARAMETER;
			goto err_unlock;
		}

		/* Check if an error occurred during discovery. */
		if (ret != 0) {
			goto err_unlock;
		}

		/* If we didn't find any matches the UUID is unknown. */
		if (partition_count == 0) {
			ret = FFA_ERROR_INVALID_PARAMETER;
			goto err_unlock;
		}

		mbox->state = MAILBOX_STATE_FULL;
		spin_unlock(&mbox->lock);
	}
	SMC_RET3(handle, FFA_SUCCESS_SMC32, 0, partition_count);

err_unlock:
	spin_unlock(&mbox->lock);
	return spmc_ffa_error_return(handle, ret);
}

static uint64_t ffa_features_handler(uint32_t smc_fid,
				     bool secure_origin,
				     uint64_t x1,
				     uint64_t x2,
				     uint64_t x3,
				     uint64_t x4,
				     void *cookie,
				     void *handle,
				     uint64_t flags)
{
	uint32_t function_id = x1;
	uint32_t input_properties = x2;

	/*
	 * We don't currently support any additional input properties
	 * for any ABI therefore ensure this value is always set to 0.
	 */
	if (input_properties != 0) {
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	if ((function_id & FFA_FEATURES_BIT31_MASK)) {
		/* Report if an FFA ABI is supported. */
		switch (function_id) {
		/* Supported features from both worlds.*/
		case FFA_ERROR:
		case FFA_SUCCESS_SMC32:
		case FFA_INTERRUPT:
		case FFA_SPM_ID_GET:
		case FFA_ID_GET:
		case FFA_FEATURES:
		case FFA_VERSION:
		case FFA_RX_RELEASE:
		case FFA_MSG_SEND_DIRECT_REQ_SMC32:
		case FFA_MSG_SEND_DIRECT_REQ_SMC64:
		case FFA_PARTITION_INFO_GET:
		case FFA_RXTX_MAP_SMC32:
		case FFA_RXTX_MAP_SMC64:
		case FFA_RXTX_UNMAP:
		case FFA_MEM_FRAG_TX:
		case FFA_MSG_RUN:

			/*
			 * We are relying on the fact that the other registers
			 * will be set to 0 as these values align with the
			 * currently implemented features of the SPMC. If this
			 * changes this function must be extended to handle
			 * reporting the additional functionality.
			 */

			SMC_RET1(handle, FFA_SUCCESS_SMC32);
			/* Execution stops here. */

		/* Supported features only from the secure world. */
		case FFA_SECONDARY_EP_REGISTER_SMC64:
		case FFA_MSG_SEND_DIRECT_RESP_SMC32:
		case FFA_MSG_SEND_DIRECT_RESP_SMC64:
		case FFA_MEM_RETRIEVE_REQ_SMC32:
		case FFA_MEM_RETRIEVE_REQ_SMC64:
		case FFA_MEM_RELINQUISH:
		case FFA_MSG_WAIT:

			if (!secure_origin) {
				return spmc_ffa_error_return(handle,
						FFA_ERROR_NOT_SUPPORTED);
			}
			SMC_RET1(handle, FFA_SUCCESS_SMC32);
			/* Execution stops here. */


		/* Supported features only from the normal world. */
		case FFA_MEM_SHARE_SMC32:
		case FFA_MEM_SHARE_SMC64:
		case FFA_MEM_LEND_SMC32:
		case FFA_MEM_LEND_SMC64:
		case FFA_MEM_RECLAIM:
		case FFA_MEM_FRAG_RX:

			if (secure_origin) {
				return spmc_ffa_error_return(handle,
						FFA_ERROR_NOT_SUPPORTED);
			}
			SMC_RET1(handle, FFA_SUCCESS_SMC32);

		default:
			return spmc_ffa_error_return(handle,
						FFA_ERROR_NOT_SUPPORTED);
		}
	} else {
		/* We currently don't support any additional features. */
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}
}

static uint64_t ffa_id_get_handler(uint32_t smc_fid,
				   bool secure_origin,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	if (secure_origin) {
		SMC_RET3(handle, FFA_SUCCESS_SMC32, 0x0,
			 spmc_get_current_sp_ctx()->sp_id);
	} else {
		SMC_RET3(handle, FFA_SUCCESS_SMC32, 0x0,
			 spmc_get_hyp_ctx()->ns_ep_id);
	}
}

/*
 * Enable an SP to query the ID assigned to the SPMC.
 */
static uint64_t ffa_spm_id_get_handler(uint32_t smc_fid,
				       bool secure_origin,
				       uint64_t x1,
				       uint64_t x2,
				       uint64_t x3,
				       uint64_t x4,
				       void *cookie,
				       void *handle,
				       uint64_t flags)
{
	SMC_RET3(handle, FFA_SUCCESS_SMC32, 0x0, FFA_SPMC_ID);
}

static uint64_t ffa_run_handler(uint32_t smc_fid,
				bool secure_origin,
				uint64_t x1,
				uint64_t x2,
				uint64_t x3,
				uint64_t x4,
				void *cookie,
				void *handle,
				uint64_t flags)
{
	struct secure_partition_desc *sp;
	uint16_t target_id = FFA_RUN_EP_ID(x1);
	unsigned int idx;
	unsigned int *rt_state;
	unsigned int *rt_model;

	/* Can only be called from the normal world. */
	if (secure_origin) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Cannot run a Normal world partition. */
	if (ffa_is_normal_world_id(target_id)) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Check that the target SP exists. */
	sp = spmc_get_sp_ctx(target_id);
	if (sp == NULL) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	idx = get_ec_index(sp);
	rt_state = &((sp->ec[idx]).rt_state);
	rt_model = &((sp->ec[idx]).rt_model);
	if (*rt_state == RT_STATE_RUNNING) {
		return spmc_ffa_error_return(handle, FFA_ERROR_BUSY);
	}

	/*
	 * Sanity check that if the execution context was not waiting then it
	 * was either in the direct request or the run partition runtime model.
	 */
	if (*rt_state == RT_STATE_PREEMPTED || *rt_state == RT_STATE_BLOCKED) {
		assert(*rt_model == RT_MODEL_RUN ||
		       *rt_model == RT_MODEL_DIR_REQ);
	}

	/*
	 * If the context was waiting then update the partition runtime model.
	 */
	if (*rt_state == RT_STATE_WAITING) {
		*rt_model = RT_MODEL_RUN;
	}

	/*
	 * Forward the request to the correct SP vCPU after updating
	 * its state.
	 */
	*rt_state = RT_STATE_RUNNING;

	return spmc_smc_return(smc_fid, secure_origin, x1, 0, 0, 0,
			       handle, cookie, flags, target_id);
}

static uint64_t rx_release_handler(uint32_t smc_fid,
				   bool secure_origin,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	struct mailbox *mbox = spmc_get_mbox_desc(secure_origin);

	spin_lock(&mbox->lock);

	if (mbox->state != MAILBOX_STATE_FULL) {
		spin_unlock(&mbox->lock);
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	mbox->state = MAILBOX_STATE_EMPTY;
	spin_unlock(&mbox->lock);

	SMC_RET1(handle, FFA_SUCCESS_SMC32);
}

/*
 * Perform initial validation on the provided secondary entry point.
 * For now ensure it does not lie within the BL31 Image or the SP's
 * RX/TX buffers as these are mapped within EL3.
 * TODO: perform validation for additional invalid memory regions.
 */
static int validate_secondary_ep(uintptr_t ep, struct secure_partition_desc *sp)
{
	struct mailbox *mb;
	uintptr_t buffer_size;
	uintptr_t sp_rx_buffer;
	uintptr_t sp_tx_buffer;
	uintptr_t sp_rx_buffer_limit;
	uintptr_t sp_tx_buffer_limit;

	mb = &sp->mailbox;
	buffer_size = (uintptr_t) (mb->rxtx_page_count * FFA_PAGE_SIZE);
	sp_rx_buffer = (uintptr_t) mb->rx_buffer;
	sp_tx_buffer = (uintptr_t) mb->tx_buffer;
	sp_rx_buffer_limit = sp_rx_buffer + buffer_size;
	sp_tx_buffer_limit = sp_tx_buffer + buffer_size;

	/*
	 * Check if the entry point lies within BL31, or the
	 * SP's RX or TX buffer.
	 */
	if ((ep >= BL31_BASE && ep < BL31_LIMIT) ||
	    (ep >= sp_rx_buffer && ep < sp_rx_buffer_limit) ||
	    (ep >= sp_tx_buffer && ep < sp_tx_buffer_limit)) {
		return -EINVAL;
	}
	return 0;
}

/*******************************************************************************
 * This function handles the FFA_SECONDARY_EP_REGISTER SMC to allow an SP to
 *  register an entry point for initialization during a secondary cold boot.
 ******************************************************************************/
static uint64_t ffa_sec_ep_register_handler(uint32_t smc_fid,
					    bool secure_origin,
					    uint64_t x1,
					    uint64_t x2,
					    uint64_t x3,
					    uint64_t x4,
					    void *cookie,
					    void *handle,
					    uint64_t flags)
{
	struct secure_partition_desc *sp;
	struct sp_exec_ctx *sp_ctx;

	/* This request cannot originate from the Normal world. */
	if (!secure_origin) {
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	/* Get the context of the current SP. */
	sp = spmc_get_current_sp_ctx();
	if (sp == NULL) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Only an S-EL1 SP should be invoking this ABI. */
	if (sp->runtime_el != S_EL1) {
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Ensure the SP is in its initialization state. */
	sp_ctx = spmc_get_sp_ec(sp);
	if (sp_ctx->rt_model != RT_MODEL_INIT) {
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Perform initial validation of the secondary entry point. */
	if (validate_secondary_ep(x1, sp)) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Update the secondary entrypoint in SP context.
	 * We don't need a lock here as during partition initialization there
	 * will only be a single core online.
	 */
	sp->secondary_ep = x1;
	VERBOSE("%s: 0x%lx\n", __func__, sp->secondary_ep);

	SMC_RET1(handle, FFA_SUCCESS_SMC32);
}

/*******************************************************************************
 * This function will parse the Secure Partition Manifest. From manifest, it
 * will fetch details for preparing Secure partition image context and secure
 * partition image boot arguments if any.
 ******************************************************************************/
static int sp_manifest_parse(void *sp_manifest, int offset,
			     struct secure_partition_desc *sp,
			     entry_point_info_t *ep_info)
{
	int32_t ret, node;
	uint32_t config_32;

	/*
	 * Look for the mandatory fields that are expected to be present in
	 * the SP manifests.
	 */
	node = fdt_path_offset(sp_manifest, "/");
	if (node < 0) {
		ERROR("Did not find root node.\n");
		return node;
	}

	ret = fdt_read_uint32_array(sp_manifest, node, "uuid",
				    ARRAY_SIZE(sp->uuid), sp->uuid);
	if (ret != 0) {
		ERROR("Missing Secure Partition UUID.\n");
		return ret;
	}

	ret = fdt_read_uint32(sp_manifest, node, "exception-level", &config_32);
	if (ret != 0) {
		ERROR("Missing SP Exception Level information.\n");
		return ret;
	}

	sp->runtime_el = config_32;

	ret = fdt_read_uint32(sp_manifest, node, "ffa-version", &config_32);
	if (ret != 0) {
		ERROR("Missing Secure Partition FF-A Version.\n");
		return ret;
	}

	sp->ffa_version = config_32;

	ret = fdt_read_uint32(sp_manifest, node, "execution-state", &config_32);
	if (ret != 0) {
		ERROR("Missing Secure Partition Execution State.\n");
		return ret;
	}

	sp->execution_state = config_32;

	ret = fdt_read_uint32(sp_manifest, node,
			      "messaging-method", &config_32);
	if (ret != 0) {
		ERROR("Missing Secure Partition messaging method.\n");
		return ret;
	}

	/* Validate this entry, we currently only support direct messaging. */

	if (config_32 & ~(FFA_PARTITION_DIRECT_REQ_RECV |
			  FFA_PARTITION_DIRECT_REQ_SEND )) {
		WARN("Invalid Secure Partition messaging method (0x%x)\n",
		     config_32);
		return -EINVAL;
	}

	sp->properties = config_32;

	ret = fdt_read_uint32(sp_manifest, node,
			      "execution-ctx-count", &config_32);

	if (ret != 0) {
		ERROR("Missing SP Execution Context Count.\n");
		return ret;
	}

	/*
	 * Ensure this field is set correctly in the manifest however
	 * since this is currently a hardcoded value for S-EL1 partitions
	 * we don't need to save it here, just validate.
	 */
	if (config_32 != PLATFORM_CORE_COUNT) {
		ERROR("SP Execution Context Count (%u) must be %u.\n",
			config_32, PLATFORM_CORE_COUNT);
		return -EINVAL;
	}


	/*
	 * Look for the optional fields that are expected to be present in
	 * an SP manifest.
	 */
	ret = fdt_read_uint32(sp_manifest, node, "id", &config_32);
	if (ret != 0) {
		WARN("Missing Secure Partition ID.\n");
	} else {
		if (!is_ffa_secure_id_valid(config_32)) {
			ERROR("Invalid Secure Partition ID (0x%x).\n",
			      config_32);
			return -EINVAL;
		}
		sp->sp_id = config_32;
	}

	ret = fdt_read_uint32(sp_manifest, node,
			      "power-management-messages", &config_32);
	if (ret != 0) {
		WARN("Missing Power Management Messages entry.\n");
	} else {
		/* We currently only support CPU_OFF power messages so ensure
		 * additional messages have not been requested. Extend check
		 * when additional messages are supported,
		 */
		if (config_32 & ~FFA_PM_MSG_SUB_CPU_OFF) {
			ERROR("Requested unsupported PM messages (%x)\n",
			      config_32);
			return -EINVAL;
		}
		sp->pwr_mgmt_msgs = config_32;
	}

	return 0;
}

/*******************************************************************************
 * This function gets the Secure Partition Manifest base and maps the manifest
 * region.
 * Currently only one Secure Partition manifest is considered which is used to
 * prepare the context for the single Secure Partition.
 ******************************************************************************/
static int find_and_prepare_sp_context(void)
{
	void *sp_manifest;
	uintptr_t manifest_base;
	uintptr_t manifest_base_align;
	entry_point_info_t *next_image_ep_info;
	int32_t ret;
	struct secure_partition_desc *sp;

	next_image_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (next_image_ep_info == NULL) {
		WARN("No Secure Partition image provided by BL2.\n");
		return -ENOENT;
	}

	sp_manifest = (void *)next_image_ep_info->args.arg0;
	if (sp_manifest == NULL) {
		WARN("Secure Partition manifest absent.\n");
		return -ENOENT;
	}

	manifest_base = (uintptr_t)sp_manifest;
	manifest_base_align = page_align(manifest_base, DOWN);

	/*
	 * Map the secure partition manifest region in the EL3 translation
	 * regime.
	 * Map an area equal to (2 * PAGE_SIZE) for now. During manifest base
	 * alignment the region of 1 PAGE_SIZE from manifest align base may
	 * not completely accommodate the secure partition manifest region.
	 */
	ret = mmap_add_dynamic_region((unsigned long long)manifest_base_align,
				      manifest_base_align,
				      PAGE_SIZE * 2,
				      MT_RO_DATA);
	if (ret != 0) {
		ERROR("Error while mapping SP manifest (%d).\n", ret);
		return ret;
	}

	ret = fdt_node_offset_by_compatible(sp_manifest, -1,
					    "arm,ffa-manifest-1.0");
	if (ret < 0) {
		ERROR("Error happened in SP manifest reading.\n");
		return -EINVAL;
	}

	/*
	 * Store the size of the manifest so that it can be used later to pass
	 * the manifest as boot information later.
	 */
	next_image_ep_info->args.arg1 = fdt_totalsize(sp_manifest);
	INFO("Manifest size = %lu bytes.\n", next_image_ep_info->args.arg1);

	/*
	 * Select an SP descriptor for initialising the partition's execution
	 * context on the primary CPU.
	 */
	sp = spmc_get_current_sp_ctx();

	/* Initialize entry point information for the SP */
	SET_PARAM_HEAD(next_image_ep_info, PARAM_EP, VERSION_1,
		       SECURE | EP_ST_ENABLE);

	/* Parse the SP manifest. */
	ret = sp_manifest_parse(sp_manifest, ret, sp, next_image_ep_info);
	if (ret != 0) {
		ERROR("Error in Secure Partition manifest parsing.\n");
		return ret;
	}

	/* Check that the runtime EL in the manifest was correct. */
	if (sp->runtime_el != S_EL1) {
		ERROR("Unexpected runtime EL: %d\n", sp->runtime_el);
		return -EINVAL;
	}

	/* Perform any common initialisation. */
	spmc_sp_common_setup(sp, next_image_ep_info);

	/* Perform any initialisation specific to S-EL1 SPs. */
	spmc_el1_sp_setup(sp, next_image_ep_info);

	/* Initialize the SP context with the required ep info. */
	spmc_sp_common_ep_commit(sp, next_image_ep_info);

	return 0;
}

/*******************************************************************************
 * This function takes an SP context pointer and performs a synchronous entry
 * into it.
 ******************************************************************************/
static int32_t logical_sp_init(void)
{
	int32_t rc = 0;
	struct el3_lp_desc *el3_lp_descs;

	/* Perform initial validation of the Logical Partitions. */
	rc = el3_sp_desc_validate();
	if (rc != 0) {
		ERROR("Logical Partition validation failed!\n");
		return rc;
	}

	el3_lp_descs = get_el3_lp_array();

	INFO("Logical Secure Partition init start.\n");
	for (unsigned int i = 0U; i < EL3_LP_DESCS_COUNT; i++) {
		rc = el3_lp_descs[i].init();
		if (rc != 0) {
			ERROR("Logical SP (0x%x) Failed to Initialize\n",
			      el3_lp_descs[i].sp_id);
			return rc;
		}
		VERBOSE("Logical SP (0x%x) Initialized\n",
			      el3_lp_descs[i].sp_id);
	}

	INFO("Logical Secure Partition init completed.\n");

	return rc;
}

uint64_t spmc_sp_synchronous_entry(struct sp_exec_ctx *ec)
{
	uint64_t rc;

	assert(ec != NULL);

	/* Assign the context of the SP to this CPU */
	cm_set_context(&(ec->cpu_ctx), SECURE);

	/* Restore the context assigned above */
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	/* Invalidate TLBs at EL1. */
	tlbivmalle1();
	dsbish();

	/* Enter Secure Partition */
	rc = spm_secure_partition_enter(&ec->c_rt_ctx);

	/* Save secure state */
	cm_el1_sysregs_context_save(SECURE);

	return rc;
}

/*******************************************************************************
 * SPMC Helper Functions.
 ******************************************************************************/
static int32_t sp_init(void)
{
	uint64_t rc;
	struct secure_partition_desc *sp;
	struct sp_exec_ctx *ec;

	sp = spmc_get_current_sp_ctx();
	ec = spmc_get_sp_ec(sp);
	ec->rt_model = RT_MODEL_INIT;
	ec->rt_state = RT_STATE_RUNNING;

	INFO("Secure Partition (0x%x) init start.\n", sp->sp_id);

	rc = spmc_sp_synchronous_entry(ec);
	if (rc != 0) {
		/* Indicate SP init was not successful. */
		ERROR("SP (0x%x) failed to initialize (%lu).\n",
		      sp->sp_id, rc);
		return 0;
	}

	ec->rt_state = RT_STATE_WAITING;
	INFO("Secure Partition initialized.\n");

	return 1;
}

static void initalize_sp_descs(void)
{
	struct secure_partition_desc *sp;

	for (unsigned int i = 0U; i < SECURE_PARTITION_COUNT; i++) {
		sp = &sp_desc[i];
		sp->sp_id = INV_SP_ID;
		sp->mailbox.rx_buffer = NULL;
		sp->mailbox.tx_buffer = NULL;
		sp->mailbox.state = MAILBOX_STATE_EMPTY;
		sp->secondary_ep = 0;
	}
}

static void initalize_ns_ep_descs(void)
{
	struct ns_endpoint_desc *ns_ep;

	for (unsigned int i = 0U; i < NS_PARTITION_COUNT; i++) {
		ns_ep = &ns_ep_desc[i];
		/*
		 * Clashes with the Hypervisor ID but will not be a
		 * problem in practice.
		 */
		ns_ep->ns_ep_id = 0;
		ns_ep->ffa_version = 0;
		ns_ep->mailbox.rx_buffer = NULL;
		ns_ep->mailbox.tx_buffer = NULL;
		ns_ep->mailbox.state = MAILBOX_STATE_EMPTY;
	}
}

/*******************************************************************************
 * Initialize SPMC attributes for the SPMD.
 ******************************************************************************/
void spmc_populate_attrs(spmc_manifest_attribute_t *spmc_attrs)
{
	spmc_attrs->major_version = FFA_VERSION_MAJOR;
	spmc_attrs->minor_version = FFA_VERSION_MINOR;
	spmc_attrs->exec_state = MODE_RW_64;
	spmc_attrs->spmc_id = FFA_SPMC_ID;
}

/*******************************************************************************
 * Initialize contexts of all Secure Partitions.
 ******************************************************************************/
int32_t spmc_setup(void)
{
	int32_t ret;
	uint32_t flags;

	/* Initialize endpoint descriptors */
	initalize_sp_descs();
	initalize_ns_ep_descs();

	/*
	 * Retrieve the information of the datastore for tracking shared memory
	 * requests allocated by platform code and zero the region if available.
	 */
	ret = plat_spmc_shmem_datastore_get(&spmc_shmem_obj_state.data,
					    &spmc_shmem_obj_state.data_size);
	if (ret != 0) {
		ERROR("Failed to obtain memory descriptor backing store!\n");
		return ret;
	}
	memset(spmc_shmem_obj_state.data, 0, spmc_shmem_obj_state.data_size);

	/* Setup logical SPs. */
	ret = logical_sp_init();
	if (ret != 0) {
		ERROR("Failed to initialize Logical Partitions.\n");
		return ret;
	}

	/* Perform physical SP setup. */

	/* Disable MMU at EL1 (initialized by BL2) */
	disable_mmu_icache_el1();

	/* Initialize context of the SP */
	INFO("Secure Partition context setup start.\n");

	ret = find_and_prepare_sp_context();
	if (ret != 0) {
		ERROR("Error in SP finding and context preparation.\n");
		return ret;
	}

	/* Register power management hooks with PSCI */
	psci_register_spd_pm_hook(&spmc_pm);

	/*
	 * Register an interrupt handler for S-EL1 interrupts
	 * when generated during code executing in the
	 * non-secure state.
	 */
	flags = 0;
	set_interrupt_rm_flag(flags, NON_SECURE);
	ret = register_interrupt_type_handler(INTR_TYPE_S_EL1,
					      spmc_sp_interrupt_handler,
					      flags);
	if (ret != 0) {
		ERROR("Failed to register interrupt handler! (%d)\n", ret);
		panic();
	}

	/* Register init function for deferred init.  */
	bl31_register_bl32_init(&sp_init);

	INFO("Secure Partition setup done.\n");

	return 0;
}

/*******************************************************************************
 * Secure Partition Manager SMC handler.
 ******************************************************************************/
uint64_t spmc_smc_handler(uint32_t smc_fid,
			  bool secure_origin,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	switch (smc_fid) {

	case FFA_VERSION:
		return ffa_version_handler(smc_fid, secure_origin, x1, x2, x3,
					   x4, cookie, handle, flags);

	case FFA_SPM_ID_GET:
		return ffa_spm_id_get_handler(smc_fid, secure_origin, x1, x2,
					     x3, x4, cookie, handle, flags);

	case FFA_ID_GET:
		return ffa_id_get_handler(smc_fid, secure_origin, x1, x2, x3,
					  x4, cookie, handle, flags);

	case FFA_FEATURES:
		return ffa_features_handler(smc_fid, secure_origin, x1, x2, x3,
					    x4, cookie, handle, flags);

	case FFA_SECONDARY_EP_REGISTER_SMC64:
		return ffa_sec_ep_register_handler(smc_fid, secure_origin, x1,
						   x2, x3, x4, cookie, handle,
						   flags);

	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
	case FFA_MSG_SEND_DIRECT_REQ_SMC64:
		return direct_req_smc_handler(smc_fid, secure_origin, x1, x2,
					      x3, x4, cookie, handle, flags);

	case FFA_MSG_SEND_DIRECT_RESP_SMC32:
	case FFA_MSG_SEND_DIRECT_RESP_SMC64:
		return direct_resp_smc_handler(smc_fid, secure_origin, x1, x2,
					       x3, x4, cookie, handle, flags);

	case FFA_RXTX_MAP_SMC32:
	case FFA_RXTX_MAP_SMC64:
		return rxtx_map_handler(smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);

	case FFA_RXTX_UNMAP:
		return rxtx_unmap_handler(smc_fid, secure_origin, x1, x2, x3,
					  x4, cookie, handle, flags);

	case FFA_PARTITION_INFO_GET:
		return partition_info_get_handler(smc_fid, secure_origin, x1,
						  x2, x3, x4, cookie, handle,
						  flags);

	case FFA_RX_RELEASE:
		return rx_release_handler(smc_fid, secure_origin, x1, x2, x3,
					  x4, cookie, handle, flags);

	case FFA_MSG_WAIT:
		return msg_wait_handler(smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);

	case FFA_ERROR:
		return ffa_error_handler(smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);

	case FFA_MSG_RUN:
		return ffa_run_handler(smc_fid, secure_origin, x1, x2, x3, x4,
				       cookie, handle, flags);

	case FFA_MEM_SHARE_SMC32:
	case FFA_MEM_SHARE_SMC64:
	case FFA_MEM_LEND_SMC32:
	case FFA_MEM_LEND_SMC64:
		return spmc_ffa_mem_send(smc_fid, secure_origin, x1, x2, x3, x4,
					 cookie, handle, flags);

	case FFA_MEM_FRAG_TX:
		return spmc_ffa_mem_frag_tx(smc_fid, secure_origin, x1, x2, x3,
					    x4, cookie, handle, flags);

	case FFA_MEM_FRAG_RX:
		return spmc_ffa_mem_frag_rx(smc_fid, secure_origin, x1, x2, x3,
					    x4, cookie, handle, flags);

	case FFA_MEM_RETRIEVE_REQ_SMC32:
	case FFA_MEM_RETRIEVE_REQ_SMC64:
		return spmc_ffa_mem_retrieve_req(smc_fid, secure_origin, x1, x2,
						 x3, x4, cookie, handle, flags);

	case FFA_MEM_RELINQUISH:
		return spmc_ffa_mem_relinquish(smc_fid, secure_origin, x1, x2,
					       x3, x4, cookie, handle, flags);

	case FFA_MEM_RECLAIM:
		return spmc_ffa_mem_reclaim(smc_fid, secure_origin, x1, x2, x3,
					    x4, cookie, handle, flags);

	default:
		WARN("Unsupported FF-A call 0x%08x.\n", smc_fid);
		break;
	}
	return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
}

/*******************************************************************************
 * This function is the handler registered for S-EL1 interrupts by the SPMC. It
 * validates the interrupt and upon success arranges entry into the SP for
 * handling the interrupt.
 ******************************************************************************/
static uint64_t spmc_sp_interrupt_handler(uint32_t id,
					  uint32_t flags,
					  void *handle,
					  void *cookie)
{
	struct secure_partition_desc *sp = spmc_get_current_sp_ctx();
	struct sp_exec_ctx *ec;
	uint32_t linear_id = plat_my_core_pos();

	/* Sanity check for a NULL pointer dereference */
	assert(sp != NULL);

	/* Panic if not an S-EL1 Partition. */
	if (sp->runtime_el != S_EL1) {
		ERROR("Interrupt received for a non S-EL1 SP on core%u.\n",
		      linear_id);
		panic();
	}

	/* Obtain a reference to the SP execution context. */
	ec = spmc_get_sp_ec(sp);

	/* Ensure that the execution context is in waiting state else panic. */
	if (ec->rt_state != RT_STATE_WAITING) {
		ERROR("SP EC on core%u is not waiting (%u), it is (%u).\n",
		      linear_id, RT_STATE_WAITING, ec->rt_state);
		panic();
	}

	/* Update the runtime model and state of the partition. */
	ec->rt_model = RT_MODEL_INTR;
	ec->rt_state = RT_STATE_RUNNING;

	VERBOSE("SP (0x%x) interrupt start on core%u.\n", sp->sp_id, linear_id);

	/*
	 * Forward the interrupt to the S-EL1 SP. The interrupt ID is not
	 * populated as the SP can determine this by itself.
	 */
	return spmd_smc_switch_state(FFA_INTERRUPT, is_caller_secure(flags),
				     FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				     FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				     handle);
}
