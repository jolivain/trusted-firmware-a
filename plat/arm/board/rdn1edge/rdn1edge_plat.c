/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <sgi_base_platform_def.h>

#if defined(IMAGE_BL31)
static struct gic600_multichip_data rdn1e1_multichip_data __init = {
	.rt_owner_base = PLAT_ARM_GICD_BASE,
	.rt_owner = 0,
	.chip_count = 2,
	.chip_addrs = {
		PLAT_ARM_GICD_BASE >> 16,
		(PLAT_ARM_GICD_BASE +
		 PLAT_ARM_SGI_REMOTE_CHIP_MEM_OFFSET(1)) >> 16
	},
	.spi_ids = {
		{32, 255},
		{0, 0}
	}
};

static uintptr_t rdn1e1_multichip_gicr_frames[] = {
	PLAT_ARM_GICR_BASE,				/* Chip 0's GICR Base */
	PLAT_ARM_GICR_BASE +
		PLAT_ARM_SGI_REMOTE_CHIP_MEM_OFFSET(1),	/* Chip 1's GICR BASE */
	0U						/* Zero Termination */
};
#endif

unsigned int plat_arm_sgi_get_platform_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_ID_OFFSET)
				& SID_SYSTEM_ID_PART_NUM_MASK;
}

unsigned int plat_arm_sgi_get_config_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_CFG_OFFSET);
}

unsigned int plat_arm_sgi_get_multi_chip_mode(void)
{
	return (mmio_read_32(SID_REG_BASE + SID_NODE_ID_OFFSET) &
			SID_MULTI_CHIP_MODE_MASK) >> SID_MULTI_CHIP_MODE_SHIFT;
}

#if defined(IMAGE_BL31)
static const mmap_region_t rdn1edge_dynamic_mmap[] = {
	CSS_SGI_MAP_DEVICE_REMOTE_CHIP(1),
	SOC_CSS_MAP_DEVICE_REMOTE_CHIP(1)
};

void plat_arm_sgi_bl31_board_setup(void)
{
	int i, ret;

	if (plat_arm_sgi_get_multi_chip_mode() == 1 &&
			PLAT_ARM_CSS_NUM_CHIPS > 1) {
		INFO("BL31: Enabling support for multi-chip\n");

		for (i = 0; i < ARRAY_SIZE(rdn1edge_dynamic_mmap); i++) {
			ret = mmap_add_dynamic_region(
					rdn1edge_dynamic_mmap[i].base_pa,
					rdn1edge_dynamic_mmap[i].base_va,
					rdn1edge_dynamic_mmap[i].size,
					rdn1edge_dynamic_mmap[i].attr
					);
			if (ret != 0) {
				ERROR("Failed to add dynamic mmap entry\n");
				panic();
			}
		}

		plat_arm_override_gicr_frames(rdn1e1_multichip_gicr_frames);
		gic600_multichip_init(&rdn1e1_multichip_data);
	}
}
#endif
