/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <mt_mcdi.h>

/* Read/Write */
#define APMCU_MCUPM_MBOX_AP_READY	0
#define APMCU_MCUPM_MBOX_RESERVED_1	1
#define APMCU_MCUPM_MBOX_RESERVED_2	2
#define APMCU_MCUPM_MBOX_RESERVED_3	3
#define APMCU_MCUPM_MBOX_PWR_CTRL_EN	4
#define APMCU_MCUPM_MBOX_L3_CACHE_MODE	5
#define APMCU_MCUPM_MBOX_BUCK_MODE	6
#define APMCU_MCUPM_MBOX_ARMPLL_MODE	7
/* Read only */
#define APMCU_MCUPM_MBOX_TASK_STA	8
#define APMCU_MCUPM_MBOX_RESERVED_9	9
#define APMCU_MCUPM_MBOX_RESERVED_10	10
#define APMCU_MCUPM_MBOX_RESERVED_11	11

/* CPC mode - Read/Write */
#define APMCU_MCUPM_MBOX_WAKEUP_CPU	12

/* Mbox Slot: APMCU_MCUPM_MBOX_PWR_CTRL_EN */
#define MCUPM_MCUSYS_CTRL	(1 << 0)
#define MCUPM_BUCK_CTRL		(1 << 1)
#define MCUPM_ARMPLL_CTRL	(1 << 2)
#define MCUPM_CM_CTRL		(1 << 3)
#define MCUPM_PWR_CTRL_MASK	((1 << 4) - 1)

/* Mbox Slot: APMCU_MCUPM_MBOX_BUCK_MODE */
#define MCUPM_BUCK_NORMAL_MODE	0 /* default */
#define MCUPM_BUCK_LP_MODE	1
#define MCUPM_BUCK_OFF_MODE	2
#define NF_MCUPM_BUCK_MODE	3U

/* Mbox Slot: APMCU_MCUPM_MBOX_ARMPLL_MODE */
#define MCUPM_ARMPLL_ON		0 /* default */
#define MCUPM_ARMPLL_GATING	1
#define MCUPM_ARMPLL_OFF	2
#define NF_MCUPM_ARMPLL_MODE	3U

/* Mbox Slot: APMCU_MCUPM_MBOX_TASK_STA */
#define MCUPM_TASK_UNINIT	0
#define MCUPM_TASK_INIT		1
#define MCUPM_TASK_INIT_FINISH	2
#define MCUPM_TASK_WAIT		3
#define MCUPM_TASK_RUN		4
#define MCUPM_TASK_PAUSE	5

#define SSPM_MBOX_3_BASE	0x0c55fce0

#define MCDI_NOT_INIT		0
#define MCDI_INIT_1		1
#define MCDI_INIT_2		2
#define MCDI_INIT_DONE		3

#define __section(x)	__attribute__((__section__(x)))
static int mcdi_init_status __section("tzfw_coherent_mem");

static inline uint32_t mcdi_mbox_read(uint32_t id)
{
	return mmio_read_32(SSPM_MBOX_3_BASE + (id << 2));
}

static inline void mcdi_mbox_write(uint32_t id, uint32_t val)
{
	mmio_write_32(SSPM_MBOX_3_BASE + (id << 2), val);
}

static void mtk_mcupm_pwr_ctrl_setting(uint32_t dev)
{
	mcdi_mbox_write(APMCU_MCUPM_MBOX_PWR_CTRL_EN, dev);
}

static void mtk_set_mcupm_pll_mode(uint32_t mode)
{
	if (mode < NF_MCUPM_ARMPLL_MODE)
		mcdi_mbox_write(APMCU_MCUPM_MBOX_ARMPLL_MODE, mode);
}

static void mtk_set_mcupm_buck_mode(uint32_t mode)
{
	if (mode < NF_MCUPM_BUCK_MODE)
		mcdi_mbox_write(APMCU_MCUPM_MBOX_BUCK_MODE, mode);
}

static int mtk_mcupm_is_ready(void)
{
	unsigned int sta = mcdi_mbox_read(APMCU_MCUPM_MBOX_TASK_STA);

	return sta == MCUPM_TASK_WAIT || sta == MCUPM_TASK_INIT_FINISH;
}

static int mcdi_init_1(void)
{
	unsigned int sta = mcdi_mbox_read(APMCU_MCUPM_MBOX_TASK_STA);

	if (sta != MCUPM_TASK_INIT)
		return -1;

	mtk_set_mcupm_pll_mode(MCUPM_ARMPLL_OFF);
	mtk_set_mcupm_buck_mode(MCUPM_BUCK_OFF_MODE);

	mtk_mcupm_pwr_ctrl_setting(
			 MCUPM_MCUSYS_CTRL |
#ifdef CONFIG_MTK_CM_MGR
			 MCUPM_CM_CTRL |
#endif
			 MCUPM_BUCK_CTRL |
			 MCUPM_ARMPLL_CTRL);

	mcdi_mbox_write(APMCU_MCUPM_MBOX_AP_READY, 1);

	return 0;
}

static int mcdi_init_2(void)
{
	return mtk_mcupm_is_ready() ? 0 : -1;
}

int mcdi_try_init(void)
{
	if (mcdi_init_status == MCDI_INIT_DONE)
		return 0;

	if (mcdi_init_status == MCDI_NOT_INIT)
		mcdi_init_status = MCDI_INIT_1;

	if (mcdi_init_status == MCDI_INIT_1 && mcdi_init_1() == 0)
		mcdi_init_status = MCDI_INIT_2;

	if (mcdi_init_status == MCDI_INIT_2 && mcdi_init_2() == 0)
		mcdi_init_status = MCDI_INIT_DONE;

	return 0;
}
