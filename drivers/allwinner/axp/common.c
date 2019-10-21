/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <libfdt.h>

#include <common/debug.h>
#include <drivers/allwinner/axp.h>

int axp_check_id(void)
{
	int ret;

	ret = axp_read(0x03);
	if (ret >= 0 && (ret & 0xcf) == axp_chip_id)
		return 0;

	return ret;
}

int axp_clrsetbits(uint8_t reg, uint8_t clr_mask, uint8_t set_mask)
{
	uint8_t val;
	int ret;

	ret = axp_read(reg);
	if (ret < 0)
		return ret;

	val = (ret & ~clr_mask) | set_mask;

	return axp_write(reg, val);
}

void axp_power_off(void)
{
	/* Set "power disable control" bit */
	axp_setbits(0x32, BIT(7));
}

/*
 * Retrieve the voltage from a given regulator DTB node.
 * Both the regulator-{min,max}-microvolt properties must be present and
 * have the same value. Return that value in millivolts.
 */
static int fdt_get_regulator_millivolt(const void *fdt, int node)
{
	const fdt32_t *prop;
	uint32_t min_volt;

	prop = fdt_getprop(fdt, node, "regulator-min-microvolt", NULL);
	if (prop == NULL)
		return -EINVAL;
	min_volt = fdt32_to_cpu(*prop);

	prop = fdt_getprop(fdt, node, "regulator-max-microvolt", NULL);
	if (prop == NULL)
		return -EINVAL;

	if (fdt32_to_cpu(*prop) != min_volt)
		return -EINVAL;

	return min_volt / 1000;
}

static int setup_regulator(const void *fdt, int node,
			   const struct axp_regulator *reg)
{
	uint8_t val;
	int mvolt;

	mvolt = fdt_get_regulator_millivolt(fdt, node);
	if (mvolt < reg->min_volt || mvolt > reg->max_volt)
		return -EINVAL;

	val = (mvolt / reg->step) - (reg->min_volt / reg->step);
	if (val > reg->split)
		val = ((val - reg->split) / 2) + reg->split;

	axp_write(reg->volt_reg, val);
	axp_setbits(reg->switch_reg, BIT(reg->switch_bit));

	INFO("PMIC: %s: %s voltage: %d.%03dV\n", axp_name, reg->dt_name,
	     mvolt / 1000, mvolt % 1000);

	return 0;
}

static bool should_enable_regulator(const void *fdt, int node)
{
	if (fdt_getprop(fdt, node, "phandle", NULL) != NULL)
		return true;
	if (fdt_getprop(fdt, node, "regulator-always-on", NULL) != NULL)
		return true;
	return false;
}

void axp_setup_regulators(const void *fdt)
{
	int node;
	bool sw = false;

	if (fdt == NULL)
		return;

	/* locate the PMIC DT node, bail out if not found */
	node = fdt_node_offset_by_compatible(fdt, -1, axp_compatible);
	if (node < 0) {
		WARN("PMIC: No %s DT node, skipping setup\n", axp_compatible);
		return;
	}

	/* This applies to AXP803 only. */
	if (fdt_getprop(fdt, node, "x-powers,drive-vbus-en", NULL)) {
		axp_clrbits(0x8f, BIT(4));
		axp_setbits(0x30, BIT(2));
		INFO("PMIC: %s: Enabling DRIVEVBUS\n", axp_name);
	}

	/* descend into the "regulators" subnode */
	node = fdt_subnode_offset(fdt, node, "regulators");
	if (node < 0) {
		WARN("PMIC: No %s DT node, skipping setup\n", "regulators");
		return;
	}

	/* iterate over all regulators to find used ones */
	fdt_for_each_subnode(node, fdt, node) {
		const struct axp_regulator *reg;
		const char *name;
		int length;

		/* We only care if it's always on or referenced. */
		if (!should_enable_regulator(fdt, node))
			continue;

		name = fdt_get_name(fdt, node, &length);
		for (reg = axp_regulators; reg->dt_name; reg++) {
			if (!strncmp(name, reg->dt_name, length)) {
				setup_regulator(fdt, node, reg);
				break;
			}
		}

		/* Enable the switch last to avoid overheating. */
		if (!strcmp(name, "dc1sw") || !strcmp(name, "sw"))
			sw = true;
	}

	/*
	 * On the AXP803, if DLDO2 is enabled after DC1SW, the PMIC overheats
	 * and shuts down. So always enable DC1SW as the very last regulator.
	 */
	if (sw) {
		INFO("PMIC: %s: Enabling SW\n", axp_name);
		axp_setbits(axp_chip_id == AXP803_CHIP_ID ? 0x12 : 0x11,
			    BIT(7));
	}
}
