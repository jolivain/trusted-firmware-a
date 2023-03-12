/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <delegated_attestation.h>
#include <psa/error.h>

int plat_rmmd_get_cca_attest_token(uintptr_t buf, size_t *len,
				   uintptr_t hash, size_t hash_size)
{
	psa_status_t ret;

	ret = rss_delegated_attest_get_token((const uint8_t *)hash, hash_size,
					     (uint8_t *)buf, *len, len);

	return ret;
}
