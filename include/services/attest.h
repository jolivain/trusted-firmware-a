/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ATTEST_H
#define ATTEST_H

#include <lib/utils_def.h>

/*******************************************************************************
 * Definitions for Platform attestation.
 ******************************************************************************/
#define SHA256_DIGEST_SIZE	32U
#define SHA384_DIGEST_SIZE	48U
#define SHA512_DIGEST_SIZE	64U

/* For printing Realm attestation token hash */
#define DIGITS_PER_BYTE				2UL
#define LENGTH_OF_TERMINATING_ZERO_IN_BYTES	1UL
#define BYTES_PER_LINE_BASE			4UL

typedef struct attest_ops {
	int (*get_token)(uint8_t *buf, size_t *len, uint8_t *hash,
			 size_t hash_size);

	int (*get_key)(uint8_t *buf, size_t *len, uint8_t type, size_t size);

} attest_ops_t;

void plat_arm_attest_init(const attest_ops_t *ops_ptr);

int get_attestation_token(uint8_t *buf, size_t *len, uint8_t *hash,
			 size_t hash_size);

int get_attestation_key(uint8_t *buf, size_t *len, uint8_t type, size_t size);

#endif /* ATTEST_H */
