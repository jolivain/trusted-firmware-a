/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/pubsub.h>
#include <lib/gpt_rme/gpt_rme.h>

#include <lib/spinlock.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/rmmd_svc.h>
#include <smccc_helpers.h>
#include <lib/extensions/sve.h>
#include "rmmd_initial_context.h"
#include "rmmd_private.h"

static spinlock_t lock;

/* For printing Realm attestation token hash */
#define DIGITS_PER_BYTE				2UL
#define LENGTH_OF_TERMINATING_ZERO_IN_BYTES	1UL
#define BYTES_PER_LINE_BASE			4UL

static void print_token_hash(uint8_t *hash, size_t hash_size)
{
	size_t leftover;
	/*
	 * bytes_per_line is always a power of two, so it can be used to
	 * construct mask with it when it is necessary to count remainder.
	 *
	 */
	const size_t bytes_per_line = 1 << BYTES_PER_LINE_BASE;
	char hash_text[(1 << BYTES_PER_LINE_BASE) * DIGITS_PER_BYTE +
		LENGTH_OF_TERMINATING_ZERO_IN_BYTES];
	const char hex_chars[] = {'0', '1', '2', '3', '4', '5', '6', '7',
				  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	unsigned int i;

	for (i = 0U; i < hash_size; ++i) {
		hash_text[(i & (bytes_per_line - 1)) * DIGITS_PER_BYTE] =
			hex_chars[hash[i] >> 4];
		hash_text[(i & (bytes_per_line - 1)) * DIGITS_PER_BYTE + 1] =
			hex_chars[hash[i] & 0x0f];
		if (((i + 1) & (bytes_per_line - 1)) == 0U) {
			hash_text[bytes_per_line * DIGITS_PER_BYTE] = '\0';
			VERBOSE("hash part %u = %s\n",
				(i >> BYTES_PER_LINE_BASE) + 1, hash_text);
		}
	}

	leftover = (size_t)i & (bytes_per_line - 1);

	if (leftover != 0UL) {
		hash_text[leftover * DIGITS_PER_BYTE] = '\0';
		VERBOSE("hash part %u = %s\n", (i >> BYTES_PER_LINE_BASE) + 1,
			hash_text);
	}
}

/*
 * TODO: Have different error codes for different errors so that the caller can
 * differentiate various error cases.
 */
int rmmd_attest_get_platform_token(uint64_t buf_pa, uint64_t *buf_len, uint64_t challenge_hash_len)
{
	int err;
	uintptr_t va;
	uint8_t temp_buf[SHA512_DIGEST_SIZE];

	/*
	 * TODO: Currently we dont validate incoming buf_pa. This is a prototype
	 * implementation and we will need to allocate static buffer for EL3-RMM
	 * communication.
	 */

	/* We need a page of buffer to pass data */
	if (*buf_len != PAGE_SIZE) {
		ERROR("Invalid buffer length\n");
		return -1;
	}

	if ((challenge_hash_len != SHA256_DIGEST_SIZE) &&
	    (challenge_hash_len != SHA384_DIGEST_SIZE) &&
	    (challenge_hash_len != SHA512_DIGEST_SIZE)) {
		ERROR("Invalid hash size: %lu\n", challenge_hash_len);
		return -1;
	}

	spin_lock(&lock);

	/* Map the buffer that was provided by the RMM. */
	err = mmap_add_dynamic_region_alloc_va(buf_pa, &va, PAGE_SIZE,
					       MT_RW_DATA | MT_REALM);
	if (err != 0) {
		ERROR("mmap_add_dynamic_region_alloc_va failed: %d (%p).\n"
		      , err, (void *)buf_pa);
		spin_unlock(&lock);
		return -1;
	}

	memcpy(temp_buf, (void *)va, challenge_hash_len);

	/*
	 * Get the hash of the Realm token from the SMC parameters's
	 * platform token buffer. In a later version this will be used
	 * as an input for the platform token generation. However for
	 * now only print it.
	 */
	print_token_hash((uint8_t *)temp_buf, challenge_hash_len);

	/* Get the platform token. */
	err = plat_get_cca_attest_token(va,
		buf_len, (uintptr_t)temp_buf, challenge_hash_len);

	if (err != 0) {
		ERROR("Failed to get platform token: %d.\n", err);
		mmap_remove_dynamic_region(va, PAGE_SIZE);
		spin_unlock(&lock);
		return -1;
	}

	/* Unmap RMM memory. */
	err = mmap_remove_dynamic_region(va, PAGE_SIZE);
	spin_unlock(&lock);
	if (err != 0) {
		ERROR("mmap_remove_dynamic_region failed: %d (%p).\n",
		      err, (void *)buf_pa);
		return -1;
	}

	return SMC_OK;
}

