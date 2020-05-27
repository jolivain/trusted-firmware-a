/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* common headers */
#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

/* mediatek platform specific headers */

/* Temporary hacks */
typedef struct {
	uint32_t val;
	uint32_t set;
	uint32_t rst;
	uint32_t align;
} GpioValRegs;

typedef struct {
	GpioValRegs dir[7];
	uint8_t rsv00[144];
	GpioValRegs dout[7];
	uint8_t rsv01[144];
	GpioValRegs din[7];
} GpioRegs;

enum {
	MAX_GPIO_REG_BITS = 32,
	MAX_EINT_REG_BITS = 32,
};

static GpioRegs *gpio_reg = (GpioRegs *)0x10005000;

static int mt_set_gpio_out(uint32_t pin, uint32_t output)
{
	uint32_t pos, bit;
	GpioRegs *reg = gpio_reg;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	uintptr_t addr = (uintptr_t)&reg->dout[pos].rst;
	ERROR("addr=%lx pos=%u,d bit=%u\n", addr, pos, bit);

	if (output == 0)
		mmio_write_32((uintptr_t)&reg->dout[pos].rst, 1L << bit);
	else
		mmio_write_32((uintptr_t)&reg->dout[pos].set, 1L << bit);

	return 0;
}

static void __dead2 plat_system_reset(void)
{
	ERROR("MTK System Reset\n");
	mt_set_gpio_out(148, 1); /* AP_EC_WARM_RST_REQ = CAM_PDN2 */
	ERROR("MTK System Reset - mmio done, going to wfi\n");
	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static const plat_psci_ops_t plat_plat_pm_ops = {
	.system_reset = plat_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_plat_pm_ops;

	return 0;
}
