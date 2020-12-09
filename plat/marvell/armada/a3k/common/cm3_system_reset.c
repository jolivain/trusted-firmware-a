/*
 * Copyright (C) 2020 Marek Behun, CZ.NIC
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>
#include <mvebu_def.h>

/* Cortex-M3 Secure Processor Mailbox Registers */
#define MVEBU_RWTM_PARAM0_REG			(MVEBU_RWTM_REG_BASE)
#define MVEBU_RWTM_CMD_REG			(MVEBU_RWTM_REG_BASE + 0x40)
#define MVEBU_RWTM_HOST_INT_RESET_REG		(MVEBU_RWTM_REG_BASE + 0xC8)
#define MVEBU_RWTM_HOST_INT_MASK_REG		(MVEBU_RWTM_REG_BASE + 0xCC)
#define MVEBU_RWTM_HOST_INT_SP_COMPLETE		BIT(0)

#define MVEBU_RWTM_REBOOT_CMD		0x0009
#define MVEBU_RWTM_REBOOT_MAGIC		0xDEADBEEF

static inline int rwtm_completed(void)
{
	return (mmio_read_32(MVEBU_RWTM_HOST_INT_RESET_REG) &
		MVEBU_RWTM_HOST_INT_SP_COMPLETE) != 0;
}

static int rwtm_wait(int ms)
{
	while (ms && !rwtm_completed()) {
		mdelay(1);
		--ms;
	}

	return rwtm_completed();
}

void cm3_system_reset(void)
{
	int tries = 5;

	for (; tries > 0; --tries) {
		mmio_clrbits_32(MVEBU_RWTM_HOST_INT_RESET_REG,
				MVEBU_RWTM_HOST_INT_SP_COMPLETE);

		mmio_write_32(MVEBU_RWTM_PARAM0_REG, MVEBU_RWTM_REBOOT_MAGIC);
		mmio_write_32(MVEBU_RWTM_CMD_REG, MVEBU_RWTM_REBOOT_CMD);

		if (rwtm_wait(10) != 0)
			break;

		mdelay(100);
	}
}
