/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/sp805.h>
#include <bl31/interrupt_mgmt.h>
#include <plat/common/platform.h>
#include <platform_def.h>

int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	/*
	 * As of now, there are no sources of Group0 secure interrupt enabled
	 * for FVP.
	 */
	return -1;
}
