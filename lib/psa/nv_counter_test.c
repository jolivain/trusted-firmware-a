/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rss_platform_api.h"

#include <stdint.h>
#include <stdio.h>

#include <drivers/arm/rss_comms.h>

int nv_counter_test(void)
{
	psa_status_t status;
	uint32_t old_val;
	uint32_t new_val;
	uint32_t id;

	status = rss_comms_init(0x2A840000UL, 0x2A850000UL);
	if (status) {
		printf("Failed at %s:%d with status %d\r\n", __func__, __LINE__, status);
		return 5;
	}

	printf("Got to %s:%d\r\n", __func__, __LINE__);

	for (id = 0; id < 3; id++) {
		status = rss_platform_nv_counter_read(id, sizeof(old_val), (uint8_t *)&old_val);
		if (status != PSA_SUCCESS) {
			printf("Failed at %s:%d with status %d\r\n", __func__, __LINE__, status);
			return 1;
		}

		printf("Got to %s:%d\r\n", __func__, __LINE__);

		status = rss_platform_nv_counter_increment(id);
		if (status != PSA_SUCCESS) {
			printf("Failed at %s:%d with status %d\r\n", __func__, __LINE__, status);
			return 2;
		}

		printf("Got to %s:%d\r\n", __func__, __LINE__);

		status = rss_platform_nv_counter_read(id, sizeof(new_val), (uint8_t *)&new_val);
		if (status != PSA_SUCCESS) {
			printf("Failed at %s:%d with status %d\r\n", __func__, __LINE__, status);
			return 3;
		}

		printf("Got to %s:%d\r\n", __func__, __LINE__);

		if(old_val + 1 != new_val) {
			printf("Failed at %s:%d", __func__, __LINE__);
			return 4;
		}
	}

	printf("Got to %s:%d\r\n", __func__, __LINE__);

	return 0;
}
