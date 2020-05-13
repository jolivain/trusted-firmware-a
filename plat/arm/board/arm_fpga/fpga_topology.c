/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include "fpga_private.h"
#include <platform_def.h>
#include <plat/common/platform.h>

#include <common/debug.h>
#include <lib/spinlock.h>

unsigned char fpga_power_domain_tree_desc[FPGA_MAX_CLUSTER_COUNT +
					  FPGA_MAX_CPUS_COUNT + 2];
unsigned char fpga_valid_mpids[PLATFORM_CORE_COUNT];

bool plat_is_my_cpu_primary(void);
unsigned int plat_fpga_calc_core_pos(uint32_t mpid);

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	unsigned int i;

	/*
	* The highest level is the system level. The next level is constituted
	* by clusters and then cores in clusters.
	*
	* This description of the power domain topology is aligned with the CPU
	* indices returned by the plat_core_pos_by_mpidr() and plat_my_core_pos()
	* APIs.
	*
	* A description of the topology tree can be found at
	* https://trustedfirmware-a.readthedocs.io/en/latest/design/psci-pd-tree.html#design
	*/

	if (fpga_power_domain_tree_desc[0] == 0U) {
		/*
		 * As fpga_power_domain_tree_desc[0] == 0, assume that the
		 * Power Domain Topology Tree has not been initialized, so
		 * perform the initialization here.
		 */

		if (!plat_is_my_cpu_primary()) {
			/* Only the primary CPU can build the tree */
			panic();
		}

		fpga_power_domain_tree_desc[0] = 1U;
		fpga_power_domain_tree_desc[1] = FPGA_MAX_CLUSTER_COUNT;

		for (i = 0U; i < FPGA_MAX_CLUSTER_COUNT; i++) {
			fpga_power_domain_tree_desc[2 + i] =
						FPGA_MAX_CPUS_PER_CLUSTER;
		}

		for (i = 0U;
		     i < FPGA_MAX_CLUSTER_COUNT * FPGA_MAX_CPUS_PER_CLUSTER;
		     i++) {
			fpga_power_domain_tree_desc[2
						    + FPGA_MAX_CLUSTER_COUNT
						    + i] = FPGA_MAX_PE_PER_CPU;
		}

		/* Add this core to the VALID mpids list */
		fpga_valid_mpids[plat_my_core_pos()] = VALID_MPID;
	}

	return fpga_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	int core_pos;

	/*
	 * Check if the topology three has been initialized. If not, initialize
	 * it, as it is needed to get the valid MPIDR array
	 */
	if (fpga_power_domain_tree_desc[0] == 0U) {
		(void)plat_get_power_domain_tree_desc();
	}

	mpidr &= (MPID_MASK & ~(MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT));
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);

	/* Calculate the core position, based on the maximum topology. */
        core_pos = plat_fpga_calc_core_pos(mpidr);

        /* Check whether this core is actually present. */
        if (fpga_valid_mpids[core_pos] != VALID_MPID) {
                return -1;
	}

	return core_pos;
}
