/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <mdio.h>

static int mdio_op_status(int result)
{
	uint32_t timeout = 1000000; /* loop for 1s */
	uint32_t val;

	do {
		val = mmio_read_32(CMIC_MIIM_STAT);
		if ((val & MDIO_STAT_DONE) == result)
			return 0;

		udelay(1);
	} while (timeout--);
	return -1;
}

static int mdio_op(uint16_t busid, uint16_t phyid, uint32_t reg,
	       uint16_t val, uint8_t op)
{
	uint32_t param;
	int ret;

	mmio_write_32(CMIC_MIIM_CTRL, 0);
	ret = mdio_op_status(0);
	if (ret)
		goto err;

	param = 0;
	param |= 1 << MDIO_PARAM_INTERNAL_SEL;
	param |= (busid & MDIO_PARAM_BUSID_MASK) << MDIO_PARAM_BUSID;
	param |= (phyid & MDIO_PARAM_PHYID_MASK) << MDIO_PARAM_PHYID;
	param |= (val & MDIO_PARAM_DATA_MASK) << MDIO_PARAM_DATA;

	mmio_write_32(CMIC_MIIM_PARAM, param);

	mmio_write_32(CMIC_MIIM_ADDRESS, reg);

	mmio_write_32(CMIC_MIIM_CTRL, op);

	ret = mdio_op_status(1);
	if (ret)
		goto err;

	if (op == MDIO_CTRL_READ_OP)
		ret = mmio_read_32(CMIC_MIIM_READ_DATA) & MDIO_READ_DATA_MASK;
err:
	return ret;
}

int mdio_write(uint16_t busid, uint16_t phyid, uint32_t reg, uint16_t val)
{
	int ret;

	ret = mdio_op(busid, phyid, reg, val, MDIO_CTRL_WRITE_OP);
	if (ret == -1)
		INFO("MDIO write fail\n");
	return ret;
}

int mdio_read(uint16_t busid, uint16_t phyid, uint32_t reg)
{
	int ret;

	ret = mdio_op(busid, phyid, reg, 0, MDIO_CTRL_READ_OP);
	if (ret == -1)
		INFO("MDIO read fail\n");
	return ret;
}
