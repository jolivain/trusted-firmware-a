/*
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <plat/common/platform.h>

/*
 * Defines member of structure and reserves space
 * for the next member with specified offset.
 */
/* cppcheck-suppress [misra-c2012-20.7] */
#define SET_MEMBER(member, start, end)	\
	union {				\
		member;			\
		unsigned char reserved##end[((end) - (start))];	\
	}

struct hes_attest_request_s {
	SET_MEMBER(uint8_t version, 0x0, 0x2);
	SET_MEMBER(uint16_t struct_size, 0x2, 0x4);
	SET_MEMBER(uint32_t alg_id, 0x4, 0x8);
	SET_MEMBER(uintptr_t rec_granule, 0x8, 0x10);
	SET_MEMBER(uint64_t req_ticket, 0x10, 0x18);
	SET_MEMBER(size_t hash_len, 0x18, 0x20);
	SET_MEMBER(uint8_t hash_buf[64], 0x20, 0x60);
};

struct hes_attest_response_s {
	SET_MEMBER(uint8_t version, 0x0, 0x2);
	SET_MEMBER(uint16_t struct_size, 0x2, 0x8);
	SET_MEMBER(uintptr_t rec_granule, 0x8, 0x10);
	SET_MEMBER(uint64_t req_ticket, 0x10, 0x18);
	SET_MEMBER(uint16_t sig_len, 0x18, 0x20);
	SET_MEMBER(uint8_t signature_buf[512], 0x20, 0x220);
};

static struct hes_attest_request_s hes_req = { 0 };
static bool hes_req_valid;

static const uint8_t sample_hes_public_key[] = {
	0x20, 0x11, 0xC7, 0xF0, 0x3C, 0xEE, 0x43, 0x25, 0x17, 0x6E,
	0x52, 0x4F, 0x03, 0x3C, 0x0C, 0xE1, 0xE2, 0x1A, 0x76, 0xE6,
	0xC1, 0xA4, 0xF0, 0xB8, 0x39, 0xAA, 0x1D, 0xF6, 0x1E, 0x0E,
	0x8A, 0x5C, 0x8A, 0x05, 0x74, 0x0F, 0x9B, 0x69, 0xEF, 0xA7,
	0xEB, 0x1A, 0x41, 0x85, 0xBD, 0x11, 0x7F, 0x68,
	0x20, 0x11, 0xC7, 0xF0, 0x3C, 0xEE, 0x43, 0x25, 0x17, 0x6E,
	0x52, 0x4F, 0x03, 0x3C, 0x0C, 0xE1, 0xE2, 0x1A, 0x76, 0xE6,
	0xC1, 0xA4, 0xF0, 0xB8, 0x39, 0xAA, 0x1D, 0xF6, 0x1E, 0x0E,
	0x8A, 0x5C, 0x8A, 0x05, 0x74, 0x0F, 0x9B, 0x69, 0xEF, 0xA7,
	0xEB, 0x1A, 0x41, 0x85, 0xBD, 0x11, 0x7F, 0x68,
};

/*
 * Get the hardcoded realm attestation public key. FVP does not
 * support HES, so provide fake key.
 */
int plat_rmmd_get_cca_realm_attest_pub_key(uintptr_t buf, size_t *len,
				       unsigned int type)
{
	(void)type;
	if (*len < sizeof(sample_hes_public_key)) {
		return -EINVAL;
	}

	(void)memcpy((void *)buf, (const void *)sample_hes_public_key,
		     sizeof(sample_hes_public_key));
	*len = sizeof(sample_hes_public_key);

	return 0;
}

int plat_rmmd_attest_push_request_hes(uint64_t buf_pa, uint64_t buf_size)
{

	if (hes_req_valid) {
		return -ENOMEM;
	}

	if (buf_size < sizeof(struct hes_attest_request_s)) {
		return -EINVAL;
	}

	hes_req = *(struct hes_attest_request_s *)buf_pa;

	if (hes_req.version != 0x10 ||
	    hes_req.struct_size != sizeof(struct hes_attest_request_s) ||
	    hes_req.hash_len != 48 || hes_req.rec_granule == 0x0) {
		return -EINVAL;
	}

	hes_req_valid = true;

	return 0;
}

int plat_rmmd_attest_pull_response_hes(uint64_t buf_pa, uint64_t *buf_size)
{
	if (hes_req_valid == false) {
		return -ENOMEM;
	}

	if (*buf_size < sizeof(sample_hes_public_key)) {
		return -EINVAL;
	}

	struct hes_attest_response_s *resp =
		(struct hes_attest_response_s *)buf_pa;
	resp->version = 0x10;
	resp->struct_size = sizeof(struct hes_attest_response_s);
	resp->rec_granule = hes_req.rec_granule;
	resp->req_ticket = hes_req.req_ticket;
	resp->sig_len = sizeof(sample_hes_public_key);
	/* Return public key as signature */
	memcpy(resp->signature_buf, sample_hes_public_key,
	       sizeof(sample_hes_public_key));

	*buf_size = sizeof(struct hes_attest_response_s);

	hes_req_valid = false;
	return 0;
}

