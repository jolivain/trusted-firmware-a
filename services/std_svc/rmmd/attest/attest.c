/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <services/attest.h>
#include <errno.h>

static const attest_ops_t *attest_ops;

int get_attestation_token(uint8_t *buf, size_t *len, uint8_t *hash,
			 size_t hash_size)
{
	if ((attest_ops != NULL) && (attest_ops->get_token != NULL)) {
		return attest_ops->get_token(buf, len, hash, hash_size);
	}

	return -ENOTSUP;
}

void plat_arm_attest_init(const attest_ops_t *ops_ptr)
{
	assert(ops_ptr != NULL);

	attest_ops = ops_ptr;
}
