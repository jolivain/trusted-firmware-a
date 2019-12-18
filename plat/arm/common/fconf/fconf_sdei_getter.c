/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <libfdt.h>
#include <plat/arm/common/fconf_sdei_getter.h>

struct sdei_dyn_config_t sdei_dyn_config;

int fconf_populate_sdei_dyn_config(uintptr_t config)
{
	int i, node, err;
	uint32_t private_events[PLAT_SDEI_DP_EVENT_MAX_CNT * 3];
	uint32_t shared_events[PLAT_SDEI_DS_EVENT_MAX_CNT * 3];

	const void *dtb = (void *)config;

	/* Assert the node offset points to compatible property */
	node = fdt_node_offset_by_compatible(dtb, -1, "arm,sdei-1.0");
	if (node < 0) {
		ERROR("FCONF: Can't find 'arm,sdei-1.0' compatible node in dtb\n");
		return node;
	}

	/* Read number of private mappings */
	err = fdtw_read_cells(dtb, node, "private_event_count", 1,
			&sdei_dyn_config.private_ev_cnt);
	if (err < 0) {
		WARN("FCONF: Read cell failed for 'private_event_count'\n");
		return err;
	}

	/* Check if the value is in range */
	if (sdei_dyn_config.private_ev_cnt > PLAT_SDEI_DP_EVENT_MAX_CNT) {
		WARN("FCONF: Invalid value for 'private_event_count': %d\n",
			sdei_dyn_config.private_ev_cnt);
		return err;
	}

	/* Read private mappings */
	err = fdtw_read_array(dtb, node, "private_events",
			sdei_dyn_config.private_ev_cnt * 3, &private_events);
	if (err < 0) {
		WARN("FCONF: Read cell failed for 'private_events'\n");
		return err;
	}

	/* Move data to fconf struct */
	for (i = 0; i < sdei_dyn_config.private_ev_cnt; i++) {
		sdei_dyn_config.private_ev_nums[i]  = private_events[3 * i];
		sdei_dyn_config.private_ev_intrs[i] = private_events[3 * i + 1];
		sdei_dyn_config.private_ev_flags[i] = private_events[3 * i + 2];
	}

	/* Read number of shared mappings */
	err = fdtw_read_cells(dtb, node, "shared_event_count", 1,
			&sdei_dyn_config.shared_ev_cnt);
	if (err < 0) {
		WARN("FCONF: Read cell failed for 'shared_event_count'\n");
		return err;
	}

	/* Check if the value is in range */
	if (sdei_dyn_config.shared_ev_cnt > PLAT_SDEI_DS_EVENT_MAX_CNT) {
		WARN("FCONF: Invalid value for 'shared_event_count': %d\n",
			sdei_dyn_config.shared_ev_cnt);
		return err;
	}

	/* Read shared mappings */
	err = fdtw_read_array(dtb, node, "shared_events",
			sdei_dyn_config.shared_ev_cnt * 3, &shared_events);
	if (err < 0) {
		WARN("FCONF: Read cell failed for 'shared_events'\n");
		return err;
	}

	/* Move data to fconf struct */
	for (i = 0; i < sdei_dyn_config.shared_ev_cnt; i++) {
		sdei_dyn_config.shared_ev_nums[i]  = shared_events[3 * i];
		sdei_dyn_config.shared_ev_intrs[i] = shared_events[3 * i + 1];
		sdei_dyn_config.shared_ev_flags[i] = shared_events[3 * i + 2];
	}

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, sdei, fconf_populate_sdei_dyn_config);
