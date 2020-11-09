/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include "dram_sub_func.h"
#include "rcar_def.h"

void rzg_dram_get_boot_status(uint32_t *status)
{
	*status = DRAM_BOOT_STATUS_COLD;
}

int32_t rzg_dram_update_boot_status(uint32_t status)
{
	return 0;
}
