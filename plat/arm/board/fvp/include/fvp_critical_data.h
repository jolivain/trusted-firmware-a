/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/nv_cntr_ids.h>
#include <lib/utils_def.h>

#define EVLOG_CRITICAL_DATA	"CRITICAL DATA"

/*
 * Start critical data Ids from 2^32/2 reserving Ids from 0 to (2^32 - 1)
 * for Images
 */
#define	CRITICAL_DATA_ID	U(0x80000000)

struct fvp_critical_data {

	/* platform NV counters */
	unsigned int nv_ctr[MAX_NV_CTR_IDS];

	/* append critical data */
};
