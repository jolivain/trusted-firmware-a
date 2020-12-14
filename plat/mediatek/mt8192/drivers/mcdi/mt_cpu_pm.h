/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_CPU_PM_H__
#define __MT_CPU_PM_H__

#define MCUSYS_STATUS_PDN		(1UL << 0)
#define MCUSYS_STATUS_CPUSYS_PROTECT	(1UL << 8)
#define MCUSYS_STATUS_MCUSYS_PROTECT	(1UL << 9)

/* cpu_pm function ID*/
enum mt_cpu_pm_user_id {
	MCUSYS_STATUS,
	CPC_COMMAND,
};

/* cpu_pm lp function ID */
enum mt_cpu_pm_lp_smc_id {
	LP_CPC_COMMAND,
	IRQS_REMAIN_ALLOC,
	IRQS_REMAIN_CTRL,
	IRQS_REMAIN_IRQ,
	IRQS_REMAIN_WAKEUP_CAT,
	IRQS_REMAIN_WAKEUP_SRC,
};
#endif /* __MT_CPU_PM_H__ */
