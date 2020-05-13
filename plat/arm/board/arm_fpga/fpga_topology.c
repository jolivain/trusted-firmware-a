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
unsigned char fpga_cpu_topology_tree[FPGA_MAX_CLUSTER_COUNT * \
				     FPGA_MAX_CPUS_PER_CLUSTER];
unsigned char fpga_max_cpu_affvl;
unsigned char fpga_max_th_affvl;

spinlock_t topology_tree_spinlock;

void fpga_update_affinity_boundaries(void);

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	unsigned int i, cluster_count, cpus_on_previous_cluster;
	uint64_t mpidr;
	void *src;
	void *dst;

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

	spin_lock(&topology_tree_spinlock);

	if (fpga_power_domain_tree_desc[0] == 0U) {
		/*
		 * As fpga_power_domain_tree_desc[0] == 0, assume that the
		 * first two levels have not been initialized and neither has
		 * the primary CPU information, so perform the final
		 * initialization here.
		 */

		dsbsy(); /* Ensure all writes by other cores are commited. */

		fpga_power_domain_tree_desc[0] = 1;

		/* Update the cluster count for this CPU */
		mpidr = read_mpidr_el1();
		i = MPIDR_AFFLVL2_VAL(mpidr);
		if (i >= FPGA_MAX_CLUSTER_COUNT) {
			panic();
		}
		if (++fpga_power_domain_tree_desc[i + 2] >=
		    FPGA_MAX_CPUS_PER_CLUSTER) {
			panic();
		}

		/*
		 * Update the thread count for the current CPU
		 */
		fpga_cpu_topology_tree[FPGA_MAX_CPUS_PER_CLUSTER * i +
					MPIDR_AFFLVL1_VAL(mpidr)]++;

		/*
		 * Find out the number of clusters on the system by walking
		 * through the array of clusters assuming that they are not
		 * sparse, so as soon as we find an empty one, stop.
		 */
		for (i = 0; i < FPGA_MAX_CLUSTER_COUNT; i++) {
			if (fpga_power_domain_tree_desc[i + 2] == 0U) {
				break;
			}
			fpga_power_domain_tree_desc[1]++;
		}
		if (fpga_power_domain_tree_desc[1] == 0U) {
			panic();
		}

		/*
		 * Populate the rest of the topology tree whith the threads per
		 * core.
		 */
		cluster_count = fpga_power_domain_tree_desc[1];
		cpus_on_previous_cluster = 0;
		for (i = 0; i < cluster_count; i++) {
			if (fpga_power_domain_tree_desc[2 + i] >
						FPGA_MAX_CPUS_PER_CLUSTER) {
				panic();
			}
			src = (void *)&fpga_cpu_topology_tree[
						FPGA_MAX_CPUS_PER_CLUSTER * i];
			dst = (void *)&fpga_power_domain_tree_desc[
						2 + cluster_count +
						cpus_on_previous_cluster];
			memcpy(dst, src, fpga_power_domain_tree_desc[2 + i]);
			cpus_on_previous_cluster = fpga_power_domain_tree_desc[2 + i];
		}

		fpga_update_affinity_boundaries();
	}

	spin_unlock(&topology_tree_spinlock);

	return fpga_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id, thread_id;

	/*
	 * The image running on the FPGA may or may not implement
	 * multithreading, and it shouldn't be assumed this is consistent
	 * across all CPUs.
	 * This ensures that any passed mpidr values reflect the status of the
	 * primary CPU's MT bit.
	 */
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);
	mpidr &= MPID_MASK;

	/*
	 * Check if the topology three has been initialized. If not, initialize
	 * it, as it is needed to get the correct affinity boundaries.
	 */
	if (fpga_power_domain_tree_desc[0] == 0) {
		(void)plat_get_power_domain_tree_desc();
	}

	if (mpidr & MPIDR_MT_MASK) {
		thread_id = MPIDR_AFFLVL0_VAL(mpidr);
		cpu_id = MPIDR_AFFLVL1_VAL(mpidr);
		cluster_id = MPIDR_AFFLVL2_VAL(mpidr);
	} else {
		thread_id = 0;
		cpu_id = MPIDR_AFFLVL0_VAL(mpidr);
		cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	}

	if (cluster_id >= fpga_power_domain_tree_desc[1]) {
		return -1;
	} else if (cpu_id > fpga_max_cpu_affvl) {
		return -1;
	} else if (thread_id > fpga_max_th_affvl) {
		return -1;
	}

	/* Calculate the correct core, catering for multi-threaded images */
	return (int) plat_fpga_calc_core_pos(mpidr);
}
