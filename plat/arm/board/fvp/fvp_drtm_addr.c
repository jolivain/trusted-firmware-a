/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */

#include <stdint.h>

#include <plat/common/platform.h>
#include <platform_def.h>

/*******************************************************************************
 * Check DLME region is within Non-Secure region of DRAM
 ******************************************************************************/
int plat_drtm_validate_dlme_ns_region(uintptr_t dlme_region_start,
				      size_t dlme_region_size)
{
	uintptr_t dlme_region_end = dlme_region_start + dlme_region_size - 1;

	if (dlme_region_start >= dlme_region_end) {
		return -1;
	} else if ((dlme_region_start >= ARM_NS_DRAM1_BASE) &&
		   (dlme_region_start < (ARM_NS_DRAM1_BASE +
		   ARM_NS_DRAM1_SIZE)) && (dlme_region_end >=
		   ARM_NS_DRAM1_BASE) && (dlme_region_end < (ARM_NS_DRAM1_BASE
		   + ARM_NS_DRAM1_SIZE))) {
		return 0;
	} else if ((dlme_region_start >= ARM_DRAM2_BASE) && (dlme_region_start <
		   (ARM_DRAM2_BASE + ARM_DRAM2_SIZE)) &&
		   (dlme_region_end >= ARM_DRAM2_BASE) && (dlme_region_end <
		   (ARM_DRAM2_BASE + ARM_DRAM2_SIZE))) {
		return 0;
	}

	return -1;
}
