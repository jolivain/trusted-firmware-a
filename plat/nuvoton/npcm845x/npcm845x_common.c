/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (C) 20122 Nuvoton Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <platform_def.h>


const mmap_region_t plat_arm_mmap[] = {
	MAP_DEVICE0,
	MAP_DEVICE1,
	{0}
};


#ifdef NPCM845X_DEBUG
/* Used for HW Validation of Security only */
const mmap_region_t plat_arm_mmap[] = {
	MAP_DEVICE2,
	MAP_DEVICE0,
	MAP_DEVICE1,
	MAP_SRAM_NS,
	{0}
};
#endif /* NPCM845X_DEBUG */

