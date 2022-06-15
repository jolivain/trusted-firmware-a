/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <rss_comms_protocol.h>
#include <common/debug.h>

psa_status_t rss_protocol_serialize_msg(psa_handle_t handle,
					int16_t type,
					const psa_invec *in_vec,
					uint8_t in_len,
					const psa_outvec *out_vec,
					uint8_t out_len,
					struct serialized_rss_comms_msg_t *msg,
					size_t *msg_len)
{
	psa_status_t status;

	switch (msg->header.protocol_ver) {
	case 0:
		status = rss_protocol_v0_serialize_msg(handle, type, in_vec, in_len, out_vec,
						       out_len, &msg->msg.v0, msg_len);
		if (status != PSA_SUCCESS) {
			return status;
		}
		break;
	case 1:
		status = rss_protocol_v1_serialize_msg(handle, type, in_vec, in_len, out_vec,
						       out_len, &msg->msg.v1, msg_len);
		if (status != PSA_SUCCESS) {
			return status;
		}
		break;
	default:
		return PSA_ERROR_NOT_SUPPORTED;
	}

	*msg_len += sizeof(struct serialized_rss_comms_header_t);

	return PSA_SUCCESS;
}

psa_status_t rss_protocol_deserialize_reply(psa_outvec *out_vec,
					    uint8_t out_len,
					    psa_status_t *return_val,
					    const struct serialized_rss_comms_reply_t *reply,
					    size_t reply_size)
{
	switch (reply->header.protocol_ver) {
	case 0:
		return rss_protocol_v0_deserialize_reply(out_vec, out_len, return_val,
							 &reply->reply.v0, reply_size);
	case 1:
		return rss_protocol_v1_deserialize_reply(out_vec, out_len, return_val,
							 &reply->reply.v1, reply_size);
	default:
		return PSA_ERROR_NOT_SUPPORTED;
	}

	return PSA_SUCCESS;
}
