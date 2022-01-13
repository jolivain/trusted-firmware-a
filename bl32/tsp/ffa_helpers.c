/*
 * Copyright (c) 2018-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if SPMC_AT_EL3

#include <common/debug.h>
#include "ffa_helpers.h"
#include <services/ffa_svc.h>
#include "tsp_private.h"

/**
 * Initialises the header of the given `ffa_memory_region`, not including the
 * composite memory region offset.
 */
static void ffa_memory_region_init_header(
	struct ffa_memory_region *memory_region, ffa_id_t sender,
	ffa_memory_attributes_t attributes, ffa_memory_region_flags_t flags,
	ffa_memory_handle_t handle, uint32_t tag, ffa_id_t *receivers,
	uint32_t receiver_count, ffa_memory_access_permissions_t permissions)
{
	memory_region->sender = sender;
	memory_region->attributes = attributes;
	memory_region->reserved_0 = 0;
	memory_region->flags = flags;
	memory_region->handle = handle;
	memory_region->tag = tag;
	memory_region->reserved_1 = 0;
	memory_region->receiver_count = receiver_count;
	for (int i = 0; i < receiver_count; i++) {
		memory_region->receivers[i].receiver_permissions.receiver =
			receivers[i];
		memory_region->receivers[i].receiver_permissions.permissions =
			permissions;
		memory_region->receivers[i].receiver_permissions.flags = 0;
		memory_region->receivers[i].reserved_0 = 0;
	}
}

/**
 * Initialises the given `ffa_memory_region` to be used for an
 * `FFA_MEM_RETRIEVE_REQ` by the receiver of a memory transaction.
 * TODO: Support differing attributes per receiver.
 *
 * Returns the size of the message written.
 */
uint32_t ffa_memory_retrieve_request_init(
	struct ffa_memory_region *memory_region, ffa_memory_handle_t handle,
	ffa_id_t sender, ffa_id_t *receivers, uint32_t receiver_count,
	uint32_t tag, ffa_memory_region_flags_t flags,
	enum ffa_data_access data_access,
	enum ffa_instruction_access instruction_access,
	enum ffa_memory_type type, enum ffa_memory_cacheability cacheability,
	enum ffa_memory_shareability shareability)
{
	ffa_memory_access_permissions_t permissions = 0;
	ffa_memory_attributes_t attributes = 0;

	/* Set memory region's permissions. */
	ffa_set_data_access_attr(&permissions, data_access);
	ffa_set_instruction_access_attr(&permissions, instruction_access);

	/* Set memory region's page attributes. */
	ffa_set_memory_type_attr(&attributes, type);
	ffa_set_memory_cacheability_attr(&attributes, cacheability);
	ffa_set_memory_shareability_attr(&attributes, shareability);

	ffa_memory_region_init_header(memory_region, sender, attributes, flags,
				      handle, tag, receivers, receiver_count,
				      permissions);
	/*
	 * Offset 0 in this case means that the hypervisor should allocate the
	 * address ranges. This is the only configuration supported by Hafnium,
	 * as it enforces 1:1 mappings in the stage 2 page tables.
	 */
	for (int i = 0; i < receiver_count; i++) {
		memory_region->receivers[i].composite_memory_region_offset = 0;
		memory_region->receivers[i].reserved_0 = 0;
	}

	return sizeof(struct ffa_memory_region) +
	       memory_region->receiver_count * sizeof(struct ffa_memory_access);
}

/* Relinquish access to memory region. */
bool ffa_mem_relinquish(void)
{
	tsp_args_t ret;

	ret = tsp_smc(FFA_MEM_RELINQUISH, 0, 0, 0, 0, 0, 0, 0);
	if (ffa_func_id(ret) != FFA_SUCCESS_SMC32) {
		ERROR("%s failed to relinquish memory! error: (%x) %x\n",
		      __func__, ffa_func_id(ret), ffa_error_code(ret));
		return -1;
	}
	return 0;
}

/* Retrieve memory shared by another partition. */
tsp_args_t ffa_mem_retrieve_req(uint32_t descriptor_length,
					uint32_t fragment_length)
{
	return tsp_smc(FFA_MEM_RETRIEVE_REQ_SMC32,
		      descriptor_length,
		      fragment_length,
		      0, 0, 0, 0, 0);
}

/* Retrieve the next memory descriptor fragment. */
tsp_args_t ffa_mem_frag_rx(uint64_t handle, uint32_t recv_length)
{
	return tsp_smc(FFA_MEM_FRAG_RX,
		       FFA_MEM_HANDLE_LOW(handle),
		       FFA_MEM_HANDLE_HIGH(handle),
		       recv_length,
		       0, 0, 0, 0);
}

/* Relinquish the memory region. */
bool memory_relinquish(struct ffa_mem_relinquish *m, uint64_t handle,
		       ffa_id_t *endpoints, uint32_t endpoint_count)
{
	ffa_mem_relinquish_init(m, handle, 0, endpoints, endpoint_count);
	if (ffa_mem_relinquish()) {
		return false;
	}

	return true;
}

/* Query SPMC that the rx buffer of the partition can be released. */
bool ffa_rx_release(void)
{
	tsp_args_t ret;

	ret = tsp_smc(FFA_RX_RELEASE, 0, 0, 0, 0, 0, 0, 0);
	return ret._regs[TSP_ARG0] != FFA_SUCCESS_SMC32;
}

/* Map the provided buffers with the SPMC. */

bool ffa_rxtx_map(uintptr_t send, uintptr_t recv, uint32_t pages)
{
	tsp_args_t ret = {0};

	ret = tsp_smc(FFA_RXTX_MAP_SMC64, send, recv, pages, 0, 0, 0, 0);
	return ret._regs[0] != FFA_SUCCESS_SMC32;
}

#endif /*SPMC_AT_EL3 */
