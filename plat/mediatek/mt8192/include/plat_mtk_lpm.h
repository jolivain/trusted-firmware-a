/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_MTK_LPM_H
#define PLAT_MTK_LPM_H

#include <lib/psci/psci.h>
#include <lib/utils_def.h>

#define MT_IRQ_REMAIN_MAX	32
#define MT_IRQ_REMAIN_CAT_LOG	BIT(31)

struct mt_irqremain {
	unsigned int count;
	unsigned int irqs[MT_IRQ_REMAIN_MAX];
	unsigned int wakeupsrc_cat[MT_IRQ_REMAIN_MAX];
	unsigned int wakeupsrc[MT_IRQ_REMAIN_MAX];
};

#define PLAT_RC_STATUS_READY		BIT(0)
#define PLAT_RC_STATUS_FEATURE_EN	BIT(1)
#define PLAT_RC_STATUS_UART_NONSLEEP	BIT(31)

struct mt_lpm_tz {
	int (*pwr_prompt)(int cpu, const psci_power_state_t *state);
	int (*pwr_reflect)(int cpu, const psci_power_state_t *state);

	int (*pwr_cpu_on)(int cpu, const psci_power_state_t *state);
	int (*pwr_cpu_dwn)(int cpu, const psci_power_state_t *state);

	int (*pwr_cluster_on)(int cpu, const psci_power_state_t *state);
	int (*pwr_cluster_dwn)(int cpu, const psci_power_state_t *state);

	int (*pwr_mcusys_on)(int cpu, const psci_power_state_t *state);
	int (*pwr_mcusys_on_finished)(int cpu, const psci_power_state_t *state);
	int (*pwr_mcusys_dwn)(int cpu, const psci_power_state_t *state);
};

int plat_mt_pm_register(struct mt_lpm_tz *mt_pm);
void mt_plat_cpu_pm_init(void);

#endif /* PLAT_MTK_LPM_H */
