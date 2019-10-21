/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/allwinner/axp.h>
#include <drivers/delay_timer.h>
#include <drivers/mentor/mi2cv.h>
#include <lib/mmio.h>

#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

#define AXP805_ADDR	0x36
#define AXP805_ID	0x03

static enum pmic_type {
	UNKNOWN,
	AXP805,
} pmic;

int axp_read(uint8_t reg)
{
	uint8_t val;
	int ret;

	ret = i2c_write(AXP805_ADDR, 0, 0, &reg, 1);
	if (ret)
		return ret;

	ret = i2c_read(AXP805_ADDR, 0, 0, &val, 1);
	if (ret)
		return ret;

	return val;
}

int axp_write(uint8_t reg, uint8_t val)
{
	return i2c_write(AXP805_ADDR, reg, 1, &val, 1);
}

static int axp805_probe(void)
{
	int ret;

	ret = axp_write(0xff, 0x0);
	if (ret) {
		ERROR("PMIC: Cannot put AXP805 in master mode\n");
		return -EPERM;
	}

	ret = axp_check_id();
	if (ret < 0) {
		ERROR("PMIC: Cannot communicate with AXP805\n");
		return -EPERM;
	} else if (ret) {
		ERROR("PMIC: Unknown PMIC 0x%02x detected\n", ret);
		return -EINVAL;
	}

	return 0;
}

void sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	sunxi_init_platform_r_twi(SUNXI_SOC_H6, false);
	/* initialise mi2cv driver */
	i2c_init((void *)SUNXI_R_I2C_BASE);

	if (axp805_probe())
		return;
	pmic = AXP805;
	NOTICE("PMIC: Detected AXP805 on I2C\n");
	axp_setup_regulators(fdt);
}

void sunxi_power_down(void)
{
	switch (pmic) {
	case AXP805:
		/* Re-initialise after rich OS might have used it. */
		sunxi_init_platform_r_twi(SUNXI_SOC_H6, false);
		/* initialise mi2cv driver */
		i2c_init((void *)SUNXI_R_I2C_BASE);
		axp_power_off();
		break;
	default:
		break;
	}
}
