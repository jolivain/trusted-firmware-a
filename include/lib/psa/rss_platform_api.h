/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __RSS_PLATFORM_API__
#define __RSS_PLATFORM_API__

#include <stdint.h>

#include "psa/error.h"

#define RSS_PLATFORM_API_ID_NV_READ       (1010)
#define RSS_PLATFORM_API_ID_NV_INCREMENT  (1011)

/*
 * Increments the given non-volatile (NV) counter by one
 *
 * counter_id	NV counter ID.
 *
 * PSA_SUCCESS if the value is read correctly. Otherwise,
 * 	it returns a PSA_ERROR.
 */
psa_status_t
rss_platform_nv_counter_increment(uint32_t counter_id);

/*
 * Reads the given non-volatile (NV) counter
 *
 * counter_id	NV counter ID.
 * size		Size of the buffer to store NV counter value
 * 			in bytes.
 * val		Pointer to store the current NV counter value.
 *
 * PSA_SUCCESS if the value is read correctly. Otherwise,
 * 	it returns a PSA_ERROR.
 */
psa_status_t
rss_platform_nv_counter_read(uint32_t counter_id,
		uint32_t size, uint8_t *val);

#endif /* __RSS_PLATFORM_API__ */
