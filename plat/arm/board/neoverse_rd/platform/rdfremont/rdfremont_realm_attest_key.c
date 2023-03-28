/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <plat/common/platform.h>
#include <psa/error.h>
#include <rse_delegated_attestation.h>

int plat_rmmd_get_cca_realm_attest_key(uintptr_t buf, size_t *len,
				       unsigned int type)
{
	psa_status_t ret;

	ret = delegated_attestation_get_key(buf, len, type);
	if (ret != PSA_SUCCESS) {
		ERROR("Unable to fetch CCA attestation key\n");
		return -1;
	}

	return 0;
}
