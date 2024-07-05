/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2024, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <platform_def.h>

#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <drivers/arm/gicv2.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#include <tegra_private.h>
#include <tegra_def.h>

static unsigned int tegra_target_masks[PLATFORM_CORE_COUNT];

/******************************************************************************
 * Tegra common helper to setup the GICv2 driver data.
 *****************************************************************************/
void tegra_gic_setup(const interrupt_prop_t *interrupt_props,
		     unsigned int interrupt_props_num)
{
	/*
	 * Tegra GIC configuration settings
	 */
	static gicv2_driver_data_t tegra_gic_data;

	/*
	 * Register Tegra GICv2 driver
	 */
	tegra_gic_data.gicd_base = TEGRA_GICD_BASE;
	tegra_gic_data.gicc_base = TEGRA_GICC_BASE;
	tegra_gic_data.interrupt_props = interrupt_props;
	tegra_gic_data.interrupt_props_num = interrupt_props_num;
	tegra_gic_data.target_masks = tegra_target_masks;
	tegra_gic_data.target_masks_num = ARRAY_SIZE(tegra_target_masks);
	gicv2_driver_init(&tegra_gic_data);
}

/******************************************************************************
 * Tegra common helper to initialize the GICv2 only driver.
 *****************************************************************************/
void tegra_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
	gicv2_cpuif_enable();
}

/******************************************************************************
 * Tegra common helper to disable the GICv2 CPU interface
 *****************************************************************************/
void tegra_gic_cpuif_deactivate(void)
{
	gicv2_cpuif_disable();
}

/******************************************************************************
 * Tegra common helper to initialize the per cpu distributor interface
 * in GICv2
 *****************************************************************************/
void tegra_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
	gicv2_cpuif_enable();
}

/******************************************************************************
 * Tegra helper to return supported interrupt types
 *****************************************************************************/
bool plat_ic_has_interrupt_type(unsigned int type)
{
	bool has_interrupt_type = false;

	switch (type) {
	case INTR_TYPE_EL3:
	case INTR_TYPE_S_EL1:
	case INTR_TYPE_NS:
		has_interrupt_type = true;
		break;
	default:
		/* Do nothing in default case */
		break;
	}

	return has_interrupt_type;
}

/******************************************************************************
 * Tegra helper to set the interrupt type
 *****************************************************************************/
void plat_ic_set_interrupt_type(unsigned int id, unsigned int type)
{
	unsigned int gicv2_group = 0U;

	/* Map canonical interrupt type to GICv2 type */
	switch (type) {
	case INTR_TYPE_EL3:
	case INTR_TYPE_S_EL1:
		gicv2_group = GICV2_INTR_GROUP0;
		break;
	case INTR_TYPE_NS:
		gicv2_group = GICV2_INTR_GROUP1;
		break;
	default:
		assert(false); /* Unreachable */
		break;
	}

	gicv2_set_interrupt_group(id, gicv2_group);
}
