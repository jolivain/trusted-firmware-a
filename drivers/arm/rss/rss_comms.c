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
#include <lib/spinlock.h>
#include <platform_def.h>
#include <psa/client.h>

#define TYPE_OFFSET	U(16)
#define TYPE_MASK	(0xFFFFUL << TYPE_OFFSET)
#define IN_LEN_OFFSET	U(8)
#define IN_LEN_MASK	(0xFFUL << IN_LEN_OFFSET)
#define OUT_LEN_OFFSET	U(0)
#define OUT_LEN_MASK	(0xFFUL << OUT_LEN_OFFSET)

#define PARAM_PACK(type, in_len, out_len)			  \
	(((((uint32_t)type) << TYPE_OFFSET) & TYPE_MASK)	| \
	 ((((uint32_t)in_len) << IN_LEN_OFFSET) & IN_LEN_MASK)	| \
	 ((((uint32_t)out_len) << OUT_LEN_OFFSET) & OUT_LEN_MASK))

#define PARAM_UNPACK_IN_LEN(ctrl_param) \
	((size_t)(((ctrl_param) & IN_LEN_MASK) >> IN_LEN_OFFSET))

/* Message types */
struct __packed packed_psa_call_t {
	uint8_t protocol_ver;
	uint8_t seq_num;
	uint16_t client_id;
	psa_handle_t handle;
	uint32_t ctrl_param; /* type, in_len, out_len */
	uint16_t io_size[4];
};

struct __packed packed_psa_reply_t {
	uint8_t protocol_ver;
	uint8_t seq_num;
	uint16_t client_id;
	int32_t return_val;
	uint16_t out_size[4];
};

/*
 * In the current implementation the RoT Service request that requires the
 * biggest message buffer is the RSS_ATTEST_GET_TOKEN. The maximum required
 * buffer size is calculated based on the platform-specific needs of
 * this request.
 */
#define MAX_REQUEST_PAYLOAD_SIZE	(PSA_INITIAL_ATTEST_CHALLENGE_SIZE_64 \
					 + PLAT_ATTEST_TOKEN_MAX_SIZE)

/* Buffer to store the messages to be sent/received. */
static uint8_t message_buf[MAX_REQUEST_PAYLOAD_SIZE] __aligned(4);

static spinlock_t message_lock;

static int32_t pack_params(const psa_invec *invecs,
			   size_t in_len,
			   uint8_t *buf,
			   size_t *buf_len)
{
	uint32_t i;
	size_t payload_size = 0;

	for (i = 0; i < in_len; ++i) {
		if (invecs[i].len > *buf_len - payload_size) {
			return -1;
		}
		memcpy(buf + payload_size, invecs[i].base, invecs[i].len);
		payload_size += invecs[i].len;
	}

	*buf_len = payload_size;
	return 0;
}

static int serialise_message(const struct packed_psa_call_t *msg,
			     const psa_invec *invecs,
			     uint8_t *payload_buf,
			     size_t *payload_len)
{
	size_t message_len = 0;
	size_t len;

	/* Copy the message header into the payload buffer. */
	len = sizeof(*msg);
	if (len > *payload_len) {
		ERROR("[COMMS] Message buffer too small.\n");
		return -1;
	}
	memcpy(payload_buf, (const void *)msg, len);
	message_len += len;

	/* The input data will follow the message header in the payload buffer. */
	len = *payload_len - message_len;
	if (pack_params(invecs, PARAM_UNPACK_IN_LEN(msg->ctrl_param),
			payload_buf + message_len, &len) != 0) {
		ERROR("[COMMS] Message buffer too small.\n");
		return -1;
	}
	message_len += len;

	*payload_len = message_len;
	return 0;
}

static void unpack_params(const uint8_t *buf,
			  psa_outvec *outvecs,
			  size_t out_len)
{
	uint32_t i;

	for (i = 0; i < out_len; ++i) {
		memcpy(outvecs[i].base, buf, outvecs[i].len);
		buf += outvecs[i].len;
	}
}

static void deserialise_reply(struct packed_psa_reply_t *reply,
			      psa_outvec *outvecs,
			      size_t outlen,
			      const uint8_t *message,
			      size_t message_len)
{
	uint32_t i;

	memcpy(reply, message, sizeof(*reply));

	/* Outvecs */
	for (i = 0; i < outlen; ++i) {
		outvecs[i].len = reply->out_size[i];
	}

	unpack_params(message + sizeof(*reply), outvecs, outlen);
}

psa_status_t psa_call(psa_handle_t handle, int32_t type,
		      const psa_invec *in_vec, size_t in_len,
		      psa_outvec *out_vec, size_t out_len)
{
	enum mhu_error_t err;
	static uint32_t seq_num = 1;
	struct packed_psa_call_t msg = {
		.protocol_ver = 0,
		.seq_num = seq_num,
		.client_id = 1,
		.handle = handle,
		.ctrl_param = PARAM_PACK(type, in_len, out_len),
	};

	struct packed_psa_reply_t reply = {0};
	size_t message_size;
	uint32_t i;

	/* Fill msg iovec lengths */
	for (i = 0; i < in_len; ++i) {
		msg.io_size[i] = in_vec[i].len;
	}
	for (i = 0; i < out_len; ++i) {
		msg.io_size[in_len + i] = out_vec[i].len;
	}

	spin_lock(&message_lock);

 	message_size = sizeof(message_buf);
	if (serialise_message(&msg, in_vec, message_buf, &message_size)) {
		/* Local buffer is probably too small. */
		spin_unlock(&message_lock);
		return PSA_ERROR_INSUFFICIENT_MEMORY;
	}

	err = mhu_send_data(message_buf, message_size);
	if (err != MHU_ERR_NONE) {
		spin_unlock(&message_lock);
		return PSA_ERROR_COMMUNICATION_FAILURE;
	}

	message_size = sizeof(message_buf);
	err = mhu_receive_data(message_buf, &message_size);
	if (err != MHU_ERR_NONE) {
		spin_unlock(&message_lock);
		return PSA_ERROR_COMMUNICATION_FAILURE;
	}

	deserialise_reply(&reply, out_vec, out_len, message_buf, message_size);

	spin_unlock(&message_lock);

	seq_num++;

	VERBOSE("[COMMS] Received reply\n");
	VERBOSE("protocol_ver=%d\n", reply.protocol_ver);
	VERBOSE("seq_num=%d\n", reply.seq_num);
	VERBOSE("client_id=%d\n", reply.client_id);
	VERBOSE("return_val=%d\n", reply.return_val);
	VERBOSE("out_size[0]=%d\n", reply.out_size[0]);

	return reply.return_val;
}

int rss_comms_init(uintptr_t mhu_sender_base, uintptr_t mhu_receiver_base)
{
	enum mhu_error_t err;

	err = mhu_init_sender(mhu_sender_base);
	if (err != MHU_ERR_NONE) {
		ERROR("Host to RSS MHU driver initialization failed: %d\n", err);
		return -1;
	}

	err = mhu_init_receiver(mhu_receiver_base);
	if (err != MHU_ERR_NONE) {
		ERROR("RSS to Host MHU driver initialization failed: %d\n", err);
		return -1;
	}

	return 0;
}
