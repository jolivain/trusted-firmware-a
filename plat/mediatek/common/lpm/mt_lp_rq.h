/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_LP_RQ__
#define __MT_LP_RQ__

#define MT_LP_RQ_XO_FPM		(1U << 0)
#define MT_LP_RQ_26M		(1U << 1)
#define MT_LP_RQ_INFRA		(1U << 2)
#define MT_LP_RQ_SYSPLL		(1U << 3)
#define MT_LP_RQ_DRAM		(1U << 4)
#define MT_LP_RQ_ALL		0xFFFFFFFF

struct mt_lp_resource_user {
	unsigned int umask;
	unsigned int uid;
	int (*request)(struct mt_lp_resource_user *this,
		       unsigned int resource);
	int (*release)(struct mt_lp_resource_user *this);
};

int mt_lp_resource_user_register(char *uname, struct mt_lp_resource_user *ru);
#endif /* __MT_LP_RQ__ */
