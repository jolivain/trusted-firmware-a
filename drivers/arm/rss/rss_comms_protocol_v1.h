/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __RSS_COMMS_PROTOCOL_V1_H__
#define __RSS_COMMS_PROTOCOL_V1_H__

#include "psa/client.h"
#include "cdefs.h"

struct __packed serialized_rss_comms_msg_v1_t {
	psa_handle_t handle;
	uint32_t ctrl_param;
	uint32_t io_sizes[PSA_MAX_IOVEC];
	uint64_t host_ptrs[PSA_MAX_IOVEC];
};

struct __packed serialized_rss_comms_reply_v1_t {
	int32_t return_val;
	uint32_t out_sizes[PSA_MAX_IOVEC];
};

psa_status_t rss_protocol_v1_serialize_msg(psa_handle_t handle,
					   int16_t type,
					   const psa_invec *in_vec,
					   uint8_t in_len,
					   const psa_outvec *out_vec,
					   uint8_t out_len,
					   struct serialized_rss_comms_msg_v1_t *msg,
					   size_t *msg_len);

psa_status_t rss_protocol_v1_deserialize_reply(psa_outvec *out_vec,
					       uint8_t out_len,
					       psa_status_t *return_val,
					       const struct serialized_rss_comms_reply_v1_t *reply,
					       size_t reply_size);

#endif /* __RSS_COMMS_PROTOCOL_V1_H__ */
