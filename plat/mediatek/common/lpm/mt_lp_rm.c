/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mt_lp_rm.h>
#include <stddef.h>

struct _mt_resource_manager_ {
	unsigned int constraint_num;
	struct mt_resource_manager *plat_rm;
};

static struct _mt_resource_manager_ plat_mt_rm;

int mt_lp_rm_register(struct mt_resource_manager *rm)
{
	int i;
	struct mt_resource_constraint *const *con;

	if (!rm || !rm->consts || plat_mt_rm.plat_rm) {
		return MT_RM_STATUS_BAD;
	}

	for (i = 0, con = rm->consts; *con != NULL; i++, con++) {
		if ((*con)->init) {
			(*con)->init();
		}
	}

	plat_mt_rm.plat_rm = rm;
	plat_mt_rm.constraint_num = i;

	return MT_RM_STATUS_OK;
}

int mt_lp_rm_reset_constraint(int constraint_id, int cpuid, int stateid)
{
	struct mt_resource_constraint const *con = NULL;
	int res;

	if (!plat_mt_rm.plat_rm || constraint_id < 0 ||
	    constraint_id >= plat_mt_rm.constraint_num) {
		return MT_RM_STATUS_BAD;
	}

	con = plat_mt_rm.plat_rm->consts[constraint_id];

	if (con && con->reset) {
		res = con->reset(cpuid, stateid);
	} else {
		res = MT_RM_STATUS_BAD;
	}

	return res;
}

int mt_lp_rm_find_and_run_constraint(int IsRun, int idx, int cpuid,
				     int stateid, void *priv)
{
	int i, res = MT_RM_STATUS_BAD;
	struct mt_resource_constraint *const *con;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if (!rm || (idx < 0)) {
		return res;
	}

	/* If subsys clk/mtcmos is on, add block-resource-off flag */
	if (rm->update) {
		i = rm->update(rm->consts, stateid, priv);
		if (i) {
			return i;
		}
	}

	for (i = idx, con = (rm->consts + idx); *con != NULL; i++, con++) {
		if ((*con)->is_valid(cpuid, stateid)) {
			res = i;

			if (IsRun && (*con)->run) {
				(*con)->run(cpuid, stateid);
			}

			break;
		}
	}

	return res;
}

int mt_lp_rm_do_update(int stateid, int type, void const *p)
{
	int res = MT_RM_STATUS_BAD;
	struct mt_resource_constraint *const *con;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if (!rm) {
		return res;
	}

	for (con = rm->consts; *con != NULL; con++) {
		if ((*con)->update) {
			res = (*con)->update(stateid, type, p);
			if (res != MT_RM_STATUS_OK) {
				break;
			}
		}
	}

	return res;
}
