/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include "fpga_private.h"
#include <platform_def.h>

#include <common/debug.h>
#include <lib/spinlock.h>

unsigned char fpga_power_domain_tree_desc[PLATFORM_CORE_COUNT + 2];
uint32_t fpga_mpids[PLATFORM_CORE_COUNT];

spinlock_t topology_tree_spinlock;

bool plat_is_my_cpu_primary(void);
unsigned int plat_fpga_calc_provisional_core_pos(uint32_t mpid);
int get_cpuid_by_mpidr(u_register_t mpidr);

static void fpga_generate_power_topology_tree(void)
{
	/*
	 * Iterates over fpga_mpids to generate the power domain tree
	 * description.
	 */

	int max_cluster = -1;
	unsigned int cluster_count, i, j;

	/* First pass: identify clusters and cores on the tree */
	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		unsigned int cluster;

		if (!(fpga_mpids[i] & DISCOVERED_MPID_MASK)) {
			continue;
		}

		cluster = MPIDR_AFFLVL2_VAL(fpga_mpids[i]);

		if ((int)cluster > max_cluster) {
			max_cluster = cluster;
			if (++fpga_power_domain_tree_desc[1] >
						FPGA_MAX_CLUSTER_COUNT) {
				panic();
			}
		}

		if (++fpga_power_domain_tree_desc[2 + cluster] >
						FPGA_MAX_CPUS_PER_CLUSTER) {
			panic();
		}
	}

	/* Second pass: add the threads for each core on the tree */
	cluster_count = fpga_power_domain_tree_desc[1];
	j = 0; /* CPUs on previous cluster */
	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		unsigned int cpu, cluster;

		if (!(fpga_mpids[i] & DISCOVERED_MPID_MASK)) {
			continue;
		}

		cpu = MPIDR_AFFLVL1_VAL(fpga_mpids[i]);
		cluster = MPIDR_AFFLVL2_VAL(fpga_mpids[i]);

		/*
		 * Get the position of the PE count for the current cluster:
		 * - 2: Index on the tree where afflvl1 starts (CPUs per
		 *      cluster)
		 * - cluster_count: Afflvl0 description is adjactent to afflvl1
		 * - j: Afflvl0 for the current cluster is shifted by the
		 *      number of CPUs (afflvl1) on the prvious one.
		 * - CPU: CPU shift on the current cluster.
		 */
		fpga_power_domain_tree_desc[2 + cluster_count + j + cpu]++;
		if (fpga_power_domain_tree_desc[2 + cluster_count + cpu] >
							FPGA_MAX_PE_PER_CPU) {
			panic();
		}
		j = fpga_power_domain_tree_desc[2 + cluster];
	}
}

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	uint64_t index;
	uint32_t mpidr;

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

	/* To avoid concurrent calls whilst the tree is being generated */
	spin_lock(&topology_tree_spinlock);

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

		dsbsy(); /* Ensure all writes by other cores are commited. */

		mpidr = (read_mpidr_el1() & MPID_MASK &
				~(MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT));

		/* Add this core to the mpids list */
		index = plat_fpga_calc_provisional_core_pos(mpidr);
		fpga_mpids[index] = mpidr | DISCOVERED_MPID_MASK;

		fpga_generate_power_topology_tree();

		fpga_power_domain_tree_desc[0] = 1;
	}

	spin_unlock(&topology_tree_spinlock);

	return fpga_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	/*
	 * Check if the topology three has been initialized. If not, initialize
	 * it, as it is needed to get the valid MPIDR array
	 */
	if (fpga_power_domain_tree_desc[0] == 0) {
		(void)plat_get_power_domain_tree_desc();
	}

	return get_cpuid_by_mpidr(mpidr);
}
