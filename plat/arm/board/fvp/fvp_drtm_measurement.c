/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/measured_boot/event_log/event_log.h>
#include <plat/common/platform.h>

#include <platform_def.h>

/* DRTM TPM Features */
static plat_drtm_tpm_features_t tpm_features;

plat_drtm_tpm_features_t *plat_drtm_get_tpm_features(void)
{
	/* No TPM-based hashing supported. */
	tpm_features.tpm_based_hash_support = false;

	/* Set to decided algorithm by Event Log driver */
	tpm_features.firmware_hash_algorithm = TPM_ALG_ID;

	return &tpm_features;
}
