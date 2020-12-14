/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_LP_RM__
#define __MT_LP_RM__

#define MT_RM_STATUS_OK		0
#define MT_RM_STATUS_BAD	-1
#define MT_RM_STATUS_STOP	-2

enum PLAT_MT_LPM_RC_TYPE {
	PLAT_RC_UPDATE_CONDITION,
	PLAT_RC_STATUS,
	PLAT_RC_UPDATE_REMAIN_IRQS,
	PLAT_RC_IS_FMAUDIO,
	PLAT_RC_IS_ADSP,
	PLAT_RC_ENTER_CNT,
	PLAT_RC_CLKBUF_STATUS,
	PLAT_RC_UFS_STATUS
};

enum PLAT_MT_LPM_HW_CTRL_TYPE {
	PLAT_AP_MDSRC_REQ,
	PLAT_AP_MDSRC_ACK,
	PLAT_AP_MDSRC_SETTLE
};

struct mt_resource_constraint {
	int level;
	int (*init)(void);
	int (*is_valid)(int cpu, int stateid);
	int (*update)(int stateid, int type, const void *p);
	int (*run)(int cpu, int stateid);
	int (*reset)(int cpu, int stateid);
	int (*get_status)(int type, void *priv);
	unsigned int (*allow)(int stateid);
};

struct mt_resource_manager {
	int (*update)(struct mt_resource_constraint **con,
		      int num, int stateid, void *priv);
	int (*hwctrl)(int type, int set, void *priv);
	struct mt_resource_constraint **consts;
};

#define mt_lp_rm_find_constraint(priv)\
	mt_lp_rm_find_and_run_constraint(0, 0, 0, 0, priv)

int mt_lp_resource_manager_register(struct mt_resource_manager *rm);
int mt_lp_rm_find_and_run_constraint(int IsRun, int idx,
				     int cpuid, int stateid,
				     void *priv);
int mt_lp_rm_do_hwctrl(int type, int set, void *priv);
int mt_lp_rm_do_constraint(int constraint_id, int cpuid, int stateid);
int mt_lp_rm_reset_constraint(int constraint_id, int cpuid, int stateid);
int mt_lp_rm_get_status(int type, void *priv);
unsigned int mt_lp_rm_constraint_allow(int constraint_id, int stateid);
int mt_lp_rm_do_update(int stateid, int type, void const *p);
int mt_lp_rm_do_constraint_update(int constraint_id, int stateid,
				  int type, void const *p);
int mt_lp_rm_init(void);
int mt_lp_rm_do_minimum_constraint(int cpu, int stateid);
#endif /* __MT_LP_RM__ */
