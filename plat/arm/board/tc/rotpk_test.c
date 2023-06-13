/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>

#include <drivers/arm/rss_comms.h>
#include <plat/common/platform.h>
#include <rss_crypto_defs.h>
#include <rss_platform_api.h>

int rotpk_test(void)
{
	psa_status_t status;
	uint8_t key_buf[128];
	size_t key_size;

	status = rss_comms_init(PLAT_RSS_AP_SND_MHU_BASE, PLAT_RSS_AP_RCV_MHU_BASE);
	if (status != PSA_SUCCESS) {
		printf("Failed to initialize RSS communication channel\n");
		return -1;
	}

	status = rss_platform_key_read(RSS_BUILTIN_KEY_ID_HOST_S_ROTPK, key_buf,
			sizeof(key_buf), &key_size);
	if (status != PSA_SUCCESS) {
		printf("Failed to export Secure World ROTPK\n");
		return -1;
	}

	printf("Secure ROTPK = ");
	for (int I = 0; I < key_size; I++) {
		printf("%02x", key_buf[I]);
	}
	printf("\r\n\n");

	status = rss_platform_key_read(RSS_BUILTIN_KEY_ID_HOST_NS_ROTPK, key_buf,
			sizeof(key_buf), &key_size);
	if (status != PSA_SUCCESS) {
		printf("Failed to export Non-Secure World ROTPK\n");
		return -1;
	}

	printf("Non-Secure ROTPK = ");
	for (int I = 0; I < key_size; I++) {
		printf("%02x", key_buf[I]);
	}
	printf("\r\n\n");

	status = rss_platform_key_read(RSS_BUILTIN_KEY_ID_HOST_CCA_ROTPK, key_buf,
			sizeof(key_buf), &key_size);
	if (status != PSA_SUCCESS) {
		printf("Failed to export CCA ROTPK\n");
		return -1;
	}

	printf("CCA ROTPK = ");
	for (int I = 0; I < key_size; I++) {
		printf("%02x", key_buf[I]);
	}
	printf("\r\n\n");

	printf("Passed platform test: rotpk_test\n");

	return 0;
}
