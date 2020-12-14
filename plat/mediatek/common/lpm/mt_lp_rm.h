/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LP_RM
#define MT_LP_RM

#define MT_RM_STATUS_OK		0
#define MT_RM_STATUS_BAD	-1

enum PLAT_MT_LPM_RC_TYPE {
	PLAT_RC_UPDATE_CONDITION,
	PLAT_RC_UPDATE_REMAIN_IRQS
};

struct mt_resource_constraint {
	int level;
	int (*init)(void);
	int (*is_valid)(int cpu, int stateid);
	int (*update)(int stateid, int type, const void *p);
	int (*run)(int cpu, int stateid);
	int (*reset)(int cpu, int stateid);
	unsigned int (*allow)(int stateid);
};

struct mt_resource_manager {
	int (*update)(struct mt_resource_constraint **con,
		      int stateid, void *priv);
	struct mt_resource_constraint **consts;
};

int mt_lp_rm_register(struct mt_resource_manager *rm);
int mt_lp_rm_find_and_run_constraint(int IsRun, int idx, int cpuid,
				     int stateid, void *priv);
int mt_lp_rm_reset_constraint(int constraint_id, int cpuid, int stateid);
int mt_lp_rm_do_update(int stateid, int type, void const *p);
#endif /* MT_LP_RM */
