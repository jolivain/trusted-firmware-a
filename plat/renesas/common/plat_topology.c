/*
 * Copyright (c) 2018-2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/psci/psci.h>

#include "platform_def.h"

static const unsigned char rcar_power_domain_tree_desc[] = {
	1,
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CLUSTER0_CORE_COUNT,
	PLATFORM_CLUSTER1_CORE_COUNT
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return rcar_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;
	int core_pos = -1;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) {
		goto done;
	}

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT) {
		goto done;
	}

	if (cluster_id == 0U && cpu_id >= PLATFORM_CLUSTER0_CORE_COUNT) {
		goto done;
	}

	if (cluster_id == 1U && cpu_id >= PLATFORM_CLUSTER1_CORE_COUNT) {
		goto done;
	}

	core_pos = cpu_id + cluster_id * PLATFORM_CLUSTER0_CORE_COUNT;
done:
	return core_pos;
}
