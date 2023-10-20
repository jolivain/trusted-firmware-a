/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMC_NOTIFICATIONS_H
#define SPMC_NOTIFICATIONS_H

#include <lib/spinlock.h>
#include <stdbool.h>
#include <stdint.h>

#include "spmc.h"

#define FFA_SCHEDULE_RECEIVER_SGI_ID   8
#define INVALID_VCPU_ID -1

// this is a generalized struct used for both binding and pending
// notification bits. taking suggestions for a better name.
typedef struct ffa_notifs_sender_bits {
	uint16_t sender_ffa_id;
	int16_t vcpu_id;
	// TODO(dmitriyf): is it worth it to split this struct?
	bool returned;
	uint64_t bitset;
	struct ffa_notifs_sender_bits *next;
} ffa_notifs_sender_bits_t;

typedef struct ffa_notifs_endpoint {
	uint16_t receiver_ffa_id;
	uint8_t num_vcpus;

	//TODO(dmitriyf): do we need to maintain this set twice?
	// uint64_t sp_pend_set;
	// uint64_t vm_pend_set;
	// uint64_t fw_pend_set;

	// uint64_t sp_bind_set;
	// uint64_t vm_bind_set;
	// uint64_t fw_bind_set;

	ffa_notifs_sender_bits_t *bindings;
	//TODO(dmitriyf): be more creative naming this
	ffa_notifs_sender_bits_t *pendings;

	struct ffa_notifs_endpoint *next;
	spinlock_t lock;
} ffa_notifs_endpoint_t;

#define GET_LIST_SIZE(header, list) (header.bits.list_sizes & 0x3 >> list * 2)
#define SET_LIST_SIZE(header, list, n) (header.bits.list_sizes |= n >> list * 2)
#define BUMP_LIST_SIZE(header, list) SET_LIST_SIZE(header, list, (GET_LIST_SIZE(header, list) + 1))
#define MAX_LIST_SIZE 3

union bits {
	struct _32 {
		uint32_t lo;
		uint32_t hi;
	} _32;
	uint64_t _64;
};

typedef union {
	struct {
		uint64_t more_pending : 1;
		uint64_t __reserved   : 6;
		uint64_t num_lists    : 5;
		// TODO(dmitriyf): revisit this
		uint64_t list_sizes   : 40;
	} bits;
	uint64_t raw;
} ffa_notifs_info_get_header_t;

typedef enum spmc_ffa_status {
	NO_ERROR = 0,
	NOT_FOUND = -1,
	ALREADY_EXISTS = -2,
	NOTIFICATIONS_PENDING = -3,
	NO_MEMORY = -4,
	ALREADY_BOUND = -5,
	INVALID_PARAMETERS = -6,
	NO_DATA = -7,
} spmc_ffa_status_t;

// #define for_each_

uintptr_t spmc_ffa_features_schedule_receiver_int(void *ns_ctx);
int spmc_ffa_notifications_init(struct secure_partition_desc *sp);
int spmc_ffa_notifications_init_per_pe(void);

int spmc_ffa_notification_bitmap_create(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t vmid,
			    uint32_t vcpu_count,
			    uint32_t x3,
			    uint32_t x4,
			    void *cookie,
			    void *handle,
			    uint64_t flags);

int spmc_ffa_notification_bitmap_destroy(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t vmid,
			    uint32_t x2,
			    uint32_t x3,
			    uint32_t x4,
			    void *cookie,
			    void *handle,
			    uint64_t flags);

int spmc_ffa_notification_bind(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t snd_rcv_ids,
			    uint32_t ffa_flags,
			    uint32_t bitmap_lo,
			    uint32_t bitmap_hi,
			    void *cookie,
			    void *handle,
			    uint64_t flags);

int spmc_ffa_notification_unbind(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t snd_rcv_ids,
			    uint32_t reserved,
			    uint32_t bitmap_lo,
			    uint32_t bitmap_hi,
			    void *cookie,
			    void *handle,
			    uint64_t flags);

int spmc_ffa_notification_set(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t snd_rcv_ids,
			    uint32_t ffa_flags,
			    uint32_t bitmap_lo,
			    uint32_t bitmap_hi,
			    void *cookie,
			    void *handle,
			    uint64_t flags);

int spmc_ffa_notification_get(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t receiver_id,
			    uint32_t ffa_flags,
			    uint32_t x3,
			    uint32_t x4,
			    void *cookie,
			    void *handle,
			    uint64_t flags);

int spmc_ffa_notification_info_get(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t x1,
			    uint32_t x2,
			    uint32_t x3,
			    uint32_t x4,
			    void *cookie,
			    void *handle,
			    uint64_t flags);

#endif /* SPMC_NOTIFICATIONS_H */
