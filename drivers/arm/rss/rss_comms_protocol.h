/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __RSS_COMMS_PROTOCOL_H__
#define __RSS_COMMS_PROTOCOL_H__

#include "psa/client.h"
#include "cdefs.h"

#include "rss_comms_protocol_v0.h"
#include "rss_comms_protocol_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

struct __packed serialized_rss_comms_header_t {
	uint8_t protocol_ver;
	uint8_t seq_num;
	uint16_t client_id;
};

/* MHU message passed from Host to RSS to deliver a PSA client call */
struct __packed serialized_rss_comms_msg_t {
	struct serialized_rss_comms_header_t header;
	union __packed {
		struct serialized_rss_comms_msg_v0_t v0;
		struct serialized_rss_comms_msg_v1_t v1;
	} msg;
};

/* MHU reply message to hold the PSA client reply result returned by RSS */
struct __packed serialized_rss_comms_reply_t {
	struct serialized_rss_comms_header_t header;
	union __packed {
		struct serialized_rss_comms_reply_v0_t v0;
		struct serialized_rss_comms_reply_v1_t v1;
	} reply;
};

psa_status_t rss_protocol_serialize_msg(psa_handle_t handle,
					int16_t type,
					const psa_invec *in_vec,
					uint8_t in_len,
					const psa_outvec *out_vec,
					uint8_t out_len,
					struct serialized_rss_comms_msg_t *msg,
					size_t *msg_len);

psa_status_t rss_protocol_deserialize_reply(psa_outvec *out_vec,
					    uint8_t out_len,
					    psa_status_t *return_val,
					    const struct serialized_rss_comms_reply_t *reply,
					    size_t reply_size);

#ifdef __cplusplus
}
#endif

#endif /* __RSS_COMMS_PROTOCOL_H__ */
