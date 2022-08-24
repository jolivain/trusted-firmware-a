/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_CONDIT_H
#define MT_SPM_CONDIT_H

#include <mt_lp_rm.h>
#include <mt_spm_cond_def.h>

struct idle_cond_info {
	unsigned int subsys_mask;
	uintptr_t addr;
	bool bit_flip;
	unsigned int clkmux_id;
};

#define IDLE_CG(mask, addr, bitflip, clkmux)	\
	{mask, (uintptr_t)addr, bitflip, clkmux}

struct mt_spm_cond_tables {
	char *name;
	unsigned int table_cg[PLAT_SPM_COND_MAX];
	unsigned int table_pll;
	void *priv;
};

extern unsigned int mt_spm_cond_check(int state_id,
				      const struct mt_spm_cond_tables *src,
				      const struct mt_spm_cond_tables *dest,
				      struct mt_spm_cond_tables *res);

extern int mt_spm_cond_update(struct mt_resource_constraint **con,
			      int stateid, void *priv);

#endif /* MT_SPM_CONDIT_H */
