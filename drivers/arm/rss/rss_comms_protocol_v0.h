/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __RSS_COMMS_PROTOCOL_V0_H__
#define __RSS_COMMS_PROTOCOL_V0_H__

#include "psa/client.h"
#include "cdefs.h"
#include <initial_attestation.h>
#include <platform_def.h>

/*
 * In the current implementation the RoT Service request that requires the
 * biggest message buffer is the RSS_ATTEST_GET_TOKEN. The maximum required
 * buffer size is calculated based on the platform-specific needs of
 * this request.
 */
#ifndef RSS_COMMS_PAYLOAD_MAX_SIZE
#define RSS_COMMS_PAYLOAD_MAX_SIZE	(PSA_INITIAL_ATTEST_CHALLENGE_SIZE_64 \
					 + PLAT_ATTEST_TOKEN_MAX_SIZE)
#endif

struct __packed serialized_rss_comms_msg_v0_t {
	psa_handle_t handle;
	uint32_t ctrl_param; /* type, in_len, out_len */
	uint16_t io_size[PSA_MAX_IOVEC];
	uint8_t trailer[RSS_COMMS_PAYLOAD_MAX_SIZE];
};

struct __packed serialized_rss_comms_reply_v0_t {
	int32_t return_val;
	uint16_t out_size[PSA_MAX_IOVEC];
	uint8_t trailer[RSS_COMMS_PAYLOAD_MAX_SIZE];
};

psa_status_t rss_protocol_v0_serialize_msg(psa_handle_t handle,
					   int16_t type,
					   const psa_invec *in_vec,
					   uint8_t in_len,
					   const psa_outvec *out_vec,
					   uint8_t out_len,
					   struct serialized_rss_comms_msg_v0_t *msg,
					   size_t *msg_len);

psa_status_t rss_protocol_v0_deserialize_reply(psa_outvec *out_vec,
					       uint8_t out_len,
					       psa_status_t *return_val,
					       const struct serialized_rss_comms_reply_v0_t *reply,
					       size_t reply_size);

#endif /* __RSS_COMMS_PROTOCOL_V0_H__ */
