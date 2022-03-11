/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <services/drtm_svc.h>

#include <platform_def.h>

/* DRTM TPM Features */
static plat_drtm_tpm_features_t tpm_features;

plat_drtm_tpm_features_t *plat_drtm_get_tpm_features(void)
{
	/* No TPM-based hashing supported. */
	tpm_features.tpm_based_hash_support = false;

	/*
	 * ToDo: get the hash algorithm from the available measured-boot
	 * backend that record the measurement, and then publish the
	 * hash algorithm.
	 */
	tpm_features.firmware_hash_algorithm = 0xB;

	return &tpm_features;
}

/* ToDo: Get the size from Event Log driver */
uint64_t plat_drtm_get_event_log_size(void)
{
	return 0UL;
}
