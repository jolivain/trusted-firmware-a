/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/spinlock.h>
#include <services/ffa_svc.h>
#include <smccc_helpers.h>

#include "common/debug.h"
#include "drivers/arm/gicv3.h"
#include "plat/common/platform.h"
#include "spmc.h"
#include "spmc_heap.h"
#include "spmc_notifications.h"

ffa_notifs_endpoint_t *notifs_endpoints;

spmc_ffa_status_t add_endpoint(uint16_t new_ffa_id, uint16_t new_vcpu_count)
{
	ffa_notifs_endpoint_t *prev = NULL;
	ffa_notifs_endpoint_t *cur = notifs_endpoints;

	while (cur != NULL && cur->receiver_ffa_id != new_ffa_id) {
		prev = cur;
		cur = cur->next;
	}
	if (cur && cur->receiver_ffa_id == new_ffa_id) {
		return ALREADY_EXISTS;
	}
	ffa_notifs_endpoint_t *new = spmc_calloc(1, sizeof(ffa_notifs_endpoint_t));

	if (!new)
		return NO_MEMORY;
	new->receiver_ffa_id = new_ffa_id;
	new->num_vcpus = new_vcpu_count;
	if (prev) {
		prev->next = new;
	} else {
		notifs_endpoints = new;
	}
	return NO_ERROR;
}

spmc_ffa_status_t get_endpoint(ffa_notifs_endpoint_t **endpoint_out, uint16_t ffa_id)
{
	ffa_notifs_endpoint_t *cur = notifs_endpoints;

	while (cur != NULL && cur->receiver_ffa_id != ffa_id) {
		cur = cur->next;
	}
	if (cur) {
		*endpoint_out = cur;
		return NO_ERROR;
	}
	return NOT_FOUND;
}

spmc_ffa_status_t remove_endpoint(uint16_t ffa_id)
{
	ffa_notifs_endpoint_t *prev = NULL;
	ffa_notifs_endpoint_t *cur = notifs_endpoints;

	while (cur != NULL && cur->receiver_ffa_id != ffa_id) {
		prev = cur;
		cur = cur->next;
	}
	if (!cur)
		return NOT_FOUND;
	if (!prev) {
		spmc_free(cur);
		notifs_endpoints = NULL;
		return NO_ERROR;
	}
	if (cur->pendings)
		return NOTIFICATIONS_PENDING;
	prev->next = cur->next;
	spmc_free(cur);
	return NO_ERROR;
}

bool same_notif(ffa_notifs_sender_bits_t *left, ffa_notifs_sender_bits_t *right)
{
	return (left->sender_ffa_id == right->sender_ffa_id) && (left->vcpu_id == right->vcpu_id);
}

spmc_ffa_status_t add_or_update_bitset(ffa_notifs_sender_bits_t **endpoint_bits, ffa_notifs_sender_bits_t *set_bits)
{
	ffa_notifs_sender_bits_t *prev = NULL;
	ffa_notifs_sender_bits_t *cur = *endpoint_bits;

	while (cur != NULL && !same_notif(cur, set_bits)) {
		prev = cur;
		cur = cur->next;
	}
	if (cur && same_notif(cur, set_bits)) {
		if (cur->bitset & set_bits->bitset) {
			return ALREADY_BOUND;
		}
		cur->bitset |= set_bits->bitset;
		return NO_ERROR;
	}
	ffa_notifs_sender_bits_t *new = spmc_calloc(1, sizeof(ffa_notifs_sender_bits_t));

	if (!new)
		return NO_MEMORY;
	*new = *set_bits;
	if (prev) {
		prev->next = new;
	} else {
		*endpoint_bits = new;
	}
	return NO_ERROR;
}

spmc_ffa_status_t remove_or_update_bitset(ffa_notifs_sender_bits_t **endpoint_bits, ffa_notifs_sender_bits_t *reset_bits)
{
	ffa_notifs_sender_bits_t *prev = NULL;
	ffa_notifs_sender_bits_t *cur = *endpoint_bits;

	while (cur != NULL && !same_notif(cur, reset_bits)) {
		prev = cur;
		cur = cur->next;
	}
	if (cur && same_notif(cur, reset_bits)) {
		if (!(cur->bitset & reset_bits->bitset)) {
			return INVALID_PARAMETERS;
		}
		cur->bitset &= !reset_bits->bitset;
		if (!cur->bitset) {
			spmc_free(cur);
			if (prev) {
				prev->next = NULL;
			} else {
				*endpoint_bits = NULL;
			}
		}
		return NO_ERROR;
	}
	return NOT_FOUND;
}

spmc_ffa_status_t accumulate_bits(ffa_notifs_sender_bits_t **endpoint_bits, bool clear, uint64_t *sp_bits, uint64_t *vm_bits, uint64_t *fw_bits)
{
	ffa_notifs_sender_bits_t *cur = *endpoint_bits;
	ffa_notifs_sender_bits_t *prev = NULL;

	while (cur != NULL) {
		if (FFA_IS_VM_ID(cur->sender_ffa_id)) {
			*vm_bits |= cur->bitset;
		} else if (FFA_IS_SP_ID(cur->sender_ffa_id)) {
			*sp_bits |= cur->bitset;
		} else if (FFA_IS_FW_ID(cur->sender_ffa_id)) {
			*fw_bits |= cur->bitset;
		} else {
			panic();
		}
		prev = cur;
		cur = cur->next;
		if (clear)
			spmc_free(prev);
	}
	if (clear)
		*endpoint_bits = NULL;
	return NO_ERROR;
}

uintptr_t spmc_ffa_features_schedule_receiver_int(void *ns_ctx)
{
	SMC_RET3(ns_ctx, FFA_SUCCESS_SMC32, 0, FFA_SCHEDULE_RECEIVER_SGI_ID);
}

int spmc_ffa_notifications_init(struct secure_partition_desc *sp)
{
	return add_endpoint(sp->sp_id, sizeof(sp->ec) / sizeof(struct sp_exec_ctx));
}

int spmc_ffa_notifications_init_per_pe(void)
{
	unsigned int this_pe = plat_my_core_pos();

	/*
	 * Configure Schedule Receiver SGI as Group-1 Non-Secure.
	 * This enables the Normal World to configure the interrupt further.
	 */
	gicv3_set_interrupt_type(FFA_SCHEDULE_RECEIVER_SGI_ID,
				 this_pe, INTR_GROUP1NS);

	return 0;
}

int spmc_ffa_notification_bitmap_create(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t vmid,
			    uint32_t vcpu_count,
			    uint32_t x3,
			    uint32_t x4,
			    void *cookie,
			    void *handle,
			    uint64_t flags)
{
	if (!FFA_IS_VM_ID(vmid)) {
		return spmc_ffa_error_return(handle, FFA_ERROR_INVALID_PARAMETER);
	}
	switch (add_endpoint(vmid, vcpu_count)) {
	case NO_ERROR:
		SMC_RET1(handle, FFA_SUCCESS_SMC32);
	break;
	case ALREADY_EXISTS:
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	break;
	case NO_MEMORY:
		return spmc_ffa_error_return(handle, FFA_ERROR_NO_MEMORY);
	break;
	default:
	panic();
	}
}

spmc_ffa_status_t pack_pending_notifications(ffa_notifs_endpoint_t *endpoints, ffa_notifs_info_get_header_t *header, uint16_t *out, size_t max_out)
{
	ffa_notifs_endpoint_t *cur_endpoint = endpoints;
	uint16_t *cur_out = out;
	size_t cur_id = 0;
	size_t cur_list = 0;
	bool new_list;

	// go through all the endpoints
	while (cur_endpoint != NULL) {
		new_list = true;
		// grab pending notifications
		ffa_notifs_sender_bits_t *cur_notif = cur_endpoint->pendings;
		// go through all of them
		while (cur_notif != NULL) {
			// if it's already been returned, move on
			if (cur_notif->returned)
				goto skip;
			// if our lists, or IDs are at capacity, return
			if (header->bits.num_lists == max_out || cur_id == max_out) {
				header->bits.more_pending = 1;
				return NO_ERROR;
			}
			if (new_list) {
				*cur_out++ = cur_endpoint->receiver_ffa_id;
				SET_LIST_SIZE((*header), cur_list, 0);
				new_list = false;
				header->bits.num_lists++;
				cur_id++;
				continue;
			}
			// if this is a per-CPU notification, add it
			if (cur_notif->vcpu_id != INVALID_VCPU_ID) {
				*cur_out++ = cur_notif->vcpu_id;
				BUMP_LIST_SIZE((*header), cur_list);
				cur_id++;
			}

			cur_notif->returned = true;
skip:
			cur_notif = cur_notif->next;
		}
		if (!new_list)
			cur_list++;

		cur_endpoint = cur_endpoint->next;
	}
	if (!cur_id)
		return NO_DATA;
	return NO_ERROR;
}

int spmc_ffa_notification_bitmap_destroy(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t vmid,
			    uint32_t x2,
			    uint32_t x3,
			    uint32_t x4,
			    void *cookie,
			    void *handle,
			    uint64_t flags)
{
	if (!FFA_IS_VM_ID(vmid)) {
		return spmc_ffa_error_return(handle, FFA_ERROR_INVALID_PARAMETER);
	}
	switch (remove_endpoint(vmid)) {
	case NO_ERROR:
		SMC_RET1(handle, FFA_SUCCESS_SMC32);
	break;
	case NOT_FOUND:
	case NOTIFICATIONS_PENDING:
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	break;
	default:
	  ERROR("Internal failure removing an FF-A notification endpoint.\n");
	  panic();
	};
}

int spmc_ffa_notification_bind(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t snd_rcv_ids,
			    uint32_t ffa_flags,
			    uint32_t bitmap_lo,
			    uint32_t bitmap_hi,
			    void *cookie,
			    void *handle,
			    uint64_t flags)
{
	uint16_t recv_id = snd_rcv_ids;
	uint16_t sender_id = snd_rcv_ids >> 16;
	// bool is_per_cpu = !!(ffa_flags & 0x1U);
	uint64_t bind_set = ((uint64_t)bitmap_hi) | bitmap_lo;
	ffa_notifs_endpoint_t *endpoint = NULL;

	if (!bind_set || get_endpoint(&endpoint, recv_id) == NOT_FOUND) {
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_INVALID_PARAMETER);
	}
	ffa_notifs_sender_bits_t bind_bits = {sender_id, INVALID_VCPU_ID, false, bind_set, NULL};
	ffa_notifs_sender_bits_t pend_bits = {sender_id, INVALID_VCPU_ID, false, 0, NULL};
	//spin_lock();
	switch (add_or_update_bitset(&endpoint->bindings, &bind_bits)) {
	case NO_ERROR:
		add_or_update_bitset(&endpoint->pendings, &pend_bits);
		SMC_RET1(handle, FFA_SUCCESS_SMC32);
	case ALREADY_BOUND:
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_DENIED);
	case NO_MEMORY:
	default:
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_ABORTED);
	}
}

int spmc_ffa_notification_unbind(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t snd_rcv_ids,
			    uint32_t reserved,
			    uint32_t bitmap_lo,
			    uint32_t bitmap_hi,
			    void *cookie,
			    void *handle,
			    uint64_t flags)
{
	uint16_t recv_id = snd_rcv_ids;
	uint16_t sender_id = snd_rcv_ids >> 16;
	// bool is_per_cpu = !!(ffa_flags & 0x1U);
	uint64_t unbind_set = ((uint64_t)bitmap_hi) | bitmap_lo;
	ffa_notifs_endpoint_t *endpoint = NULL;

	if (!unbind_set || get_endpoint(&endpoint, recv_id) == NOT_FOUND) {
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_INVALID_PARAMETER);
	}

	uint64_t pending_bits = 0;

	accumulate_bits(&endpoint->pendings, false, &pending_bits, &pending_bits, &pending_bits);

	if (pending_bits & unbind_set) {
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_DENIED);
	}
	if (endpoint->pendings) {
		spmc_free(endpoint->pendings);
		endpoint->pendings = NULL;
	}

	ffa_notifs_sender_bits_t bits = {sender_id, INVALID_VCPU_ID, false, unbind_set, NULL};
	//spin_lock();
	switch (remove_or_update_bitset(&endpoint->bindings, &bits)) {
	case NO_ERROR:
		SMC_RET1(handle, FFA_SUCCESS_SMC32);
	case ALREADY_BOUND:
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_DENIED);
	case NO_MEMORY:
	default:
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_ABORTED);
	}
}

int spmc_ffa_notification_set(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t snd_rcv_ids,
			    uint32_t ffa_flags,
			    uint32_t bitmap_lo,
			    uint32_t bitmap_hi,
			    void *cookie,
			    void *handle,
			    uint64_t flags)
{
	uint16_t recv_id = snd_rcv_ids;
	uint16_t sender_id = snd_rcv_ids >> 16;
	// bool is_per_cpu = !!(ffa_flags & 0x1U);
	uint64_t pend_set = ((uint64_t)bitmap_hi) << 32 | bitmap_lo;
	ffa_notifs_endpoint_t *endpoint = NULL;

	if (!pend_set || get_endpoint(&endpoint, recv_id) == NOT_FOUND) {
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_INVALID_PARAMETER);
	}
	// if (verify_pend_valid())
	ffa_notifs_sender_bits_t bits = {sender_id, INVALID_VCPU_ID, false, pend_set, NULL};
	//spin_lock();
	switch (add_or_update_bitset(&endpoint->pendings, &bits)) {
	case NO_ERROR:
		gicv3_set_interrupt_pending(FFA_SCHEDULE_RECEIVER_SGI_ID, plat_my_core_pos());
		SMC_RET1(handle, FFA_SUCCESS_SMC32);
	case ALREADY_BOUND:
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_DENIED);
	case NO_MEMORY:
	default:
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_ABORTED);
	}
}

int spmc_ffa_notification_get(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t receiver_id_vcpu,
			    uint32_t ffa_flags,
			    uint32_t x3,
			    uint32_t x4,
			    void *cookie,
			    void *handle,
			    uint64_t flags)
{
	uint16_t receiver_id = receiver_id_vcpu;
	// uint16_t receiver_vcpu = receiver_id_vcpu >> 16;
	bool get_sp_sent_notifs = ffa_flags >> 0 & 0x1;
	bool get_vm_sent_notifs = ffa_flags >> 1 & 0x1;
	bool get_spm_sent_notifs = ffa_flags >> 2 & 0x1;
	bool get_hyp_sent_notifs = ffa_flags >> 3 & 0x1;
	ffa_notifs_endpoint_t *endpoint;

	if (get_endpoint(&endpoint, receiver_id) == NOT_FOUND) {
		SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_INVALID_PARAMETER);
	}

	union bits sp = {0};
	union bits vm = {0};
	union bits fw = {0};

	switch (accumulate_bits(&endpoint->pendings, true, &sp._64, &vm._64, &fw._64)) {
	case NO_ERROR:
	if (!get_sp_sent_notifs)
		sp._64 = 0;
	if (!get_vm_sent_notifs)
		vm._64 = 0;
	if (!get_spm_sent_notifs)
		fw._32.lo = 0;
	if (!get_hyp_sent_notifs)
		fw._32.hi = 0;
	SMC_RET8(handle, FFA_SUCCESS_SMC32, 0, sp._32.lo, sp._32.hi, vm._32.lo, vm._32.hi, fw._32.lo, fw._32.hi);
	break;
	default:
	panic();
	break;
	}
}

int spmc_ffa_notification_info_get(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t x1,
			    uint32_t x2,
			    uint32_t x3,
			    uint32_t x4,
			    void *cookie,
			    void *handle,
			    uint64_t flags)
{
	ffa_notifs_info_get_header_t header = {0};
	uint16_t out[20] = {0};

	switch (pack_pending_notifications(notifs_endpoints, &header, out, 20)) {
	case NO_ERROR:
	SMC_RET8(handle, FFA_SUCCESS_SMC32, 0, header.raw, out[0], out[1], out[2], out[3], out[4]);
	break;
	case NO_DATA:
	SMC_RET3(handle, FFA_ERROR, 0, FFA_ERROR_NO_DATA);
	break;
	default:
	panic();
	}
}
