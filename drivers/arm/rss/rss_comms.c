/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/arm/mhu.h>
#include <drivers/arm/rss_comms.h>
#include <initial_attestation.h>
#include <psa/client.h>
#include <rss_comms_protocol.h>

#include <platform_def.h>

/* Declared statically to avoid using huge amounts of stack space. Maybe revisit
 * if functions not being reentrant becomes a problem.
 */
static struct serialized_rss_comms_msg_t msg;
static struct serialized_rss_comms_reply_t reply;

static uint8_t select_protocol_version(const psa_invec *in_vec, size_t in_len,
				       const psa_outvec *out_vec, size_t out_len)
{
	size_t in_size_total = 0;
	size_t out_size_total = 0;
	unsigned int i;

	for (i = 0U; i < in_len; ++i) {
		in_size_total += in_vec[i].len;
	}
	for (i = 0U; i < out_len; ++i) {
		out_size_total += out_vec[i].len;
	}

	/* Use v0 if we can pack into one message and reply, else use v1.
	 * TODO tune this with real performance numbers.
	 */
	if (in_size_total > 40 || out_size_total > 44) {
		return 1;
	} else {
		return 0;
	}
}

psa_status_t psa_call(psa_handle_t handle, int32_t type,
					  const psa_invec *in_vec, size_t in_len,
					  psa_outvec *out_vec, size_t out_len)
{
	enum mhu_error_t err;
	psa_status_t status;
	static uint8_t seq_num = 1U;
	size_t msg_size = sizeof(msg);
	size_t reply_size = sizeof(reply);
	psa_status_t return_val;

	memset(&msg, 0, sizeof(msg));
	msg.header.seq_num = seq_num,
	/* No need to distinguish callers (currently concurrent calls are not supported). */
	msg.header.client_id = 1U,
	msg.header.protocol_ver = select_protocol_version(in_vec, in_len, out_vec, out_len);

	memset(&reply, 0, sizeof(reply));

	if (type > INT16_MAX || type < INT16_MIN || in_len > PSA_MAX_IOVEC
	    || out_len > PSA_MAX_IOVEC) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	status = rss_protocol_serialize_msg(handle, type, in_vec, in_len, out_vec,
					    out_len, &msg, &msg_size);
	if (status != PSA_SUCCESS) {
		return status;
	}

	err = mhu_send_data((uint8_t *)&msg, msg_size);
	if (err != MHU_ERR_NONE) {
		return PSA_ERROR_COMMUNICATION_FAILURE;
	}

	err = mhu_receive_data((uint8_t *)&reply, &reply_size);
	if (err != MHU_ERR_NONE) {
		return PSA_ERROR_COMMUNICATION_FAILURE;
	}

	status = rss_protocol_deserialize_reply(out_vec, out_len, &return_val,
						&reply, reply_size);
	if (status != PSA_SUCCESS) {
		return status;
	}

	seq_num++;

	return return_val;
}

int rss_comms_init(uintptr_t mhu_sender_base, uintptr_t mhu_receiver_base)
{
	enum mhu_error_t err;

	err = mhu_init_sender(mhu_sender_base);
	if (err != MHU_ERR_NONE) {
		ERROR("[RSS-COMMS] Host to RSS MHU driver initialization failed: %d\n", err);
		return -1;
	}

	err = mhu_init_receiver(mhu_receiver_base);
	if (err != MHU_ERR_NONE) {
		ERROR("[RSS-COMMS] RSS to Host MHU driver initialization failed: %d\n", err);
		return -1;
	}

	return 0;
}
