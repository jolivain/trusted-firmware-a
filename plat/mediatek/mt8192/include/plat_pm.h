/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_PM_H__
#define __PLAT_PM_H__

#define MT_PLAT_PWR_STATE_CPU				(1)
#define MT_PLAT_PWR_STATE_CLUSTER			(2)
#define MT_PLAT_PWR_STATE_MCUSYS			(3)
#define MT_PLAT_PWR_STATE_SUSPEND2IDLE			(8)
#define MT_PLAT_PWR_STATE_SYSTEM_SUSPEND		(9)

#define MTK_LOCAL_STATE_RUN	0
#define MTK_LOCAL_STATE_RET	1
#define MTK_LOCAL_STATE_OFF	2

#define MTK_AFFLVL_CPU		0
#define MTK_AFFLVL_CLUSTER	1
#define MTK_AFFLVL_MCUSYS	2
#define MTK_AFFLVL_SYSTEM	3

#define IS_CLUSTER_OFF_STATE(s)		\
	is_local_state_off(s->pwr_domain_state[MTK_AFFLVL_CLUSTER])
#define IS_MCUSYS_OFF_STATE(s)		\
	is_local_state_off(s->pwr_domain_state[MTK_AFFLVL_MCUSYS])
#define IS_SYSTEM_SUSPEND_STATE(s)	\
	is_local_state_off(s->pwr_domain_state[MTK_AFFLVL_SYSTEM])

/*
 * definition platform power state menas.
 * PLAT_MT_SYSTEM_SUSPEND		- system suspend pwr level
 * PLAT_MT_CPU_SUSPEND_CLUSTER	- cluster off pwr level
 */
#define PLAT_MT_SYSTEM_SUSPEND		MTK_LOCAL_STATE_OFF
#define PLAT_MT_CPU_SUSPEND_CLUSTER	MPIDR_AFFLVL1

#define IS_PLAT_SYSTEM_SUSPEND(aff)	(aff == PLAT_MAX_PWR_LVL)
#define IS_PLAT_SYSTEM_RETENTION(aff)	(aff >= MPIDR_AFFLVL1)

#define IS_PLAT_SUSPEND2IDLE_ID(stateid)\
	(stateid == MT_PLAT_PWR_STATE_SUSPEND2IDLE)

#define IS_PLAT_SUSPEND_ID(stateid)\
	((stateid == MT_PLAT_PWR_STATE_SUSPEND2IDLE)\
	|| (stateid == MT_PLAT_PWR_STATE_SYSTEM_SUSPEND))

#endif /* __PLAT_PM_H__ */
