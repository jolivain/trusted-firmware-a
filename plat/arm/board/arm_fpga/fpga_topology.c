/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include <common/debug.h>
#include "fpga_private.h"
#include <platform_def.h>

unsigned char fpga_power_domain_tree_desc[FPGA_MAX_CLUSTER_COUNT + 2];
unsigned char fpga_max_cpu_affvl;
unsigned char fpga_max_pe_affvl;

void fpga_update_affinity_boundaries(void);

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
	*/
	if (fpga_power_domain_tree_desc[0] == 0U) {
		/*
		 * As fpga_power_domain_tree_desc[0] == 0, assume that the
		 * first two levels have not been initialized and neither has
		 * the primary CPU information, so perform the initialization
		 * here.
		 */

		dmbsy(); /* Ensure all writes by other cores are commited. */

		fpga_power_domain_tree_desc[0] = 1;

		/* Get the cluster to which this CPU belongs */
		i = fpga_get_cpu_cluster();

		if (i >= FPGA_MAX_CLUSTER_COUNT) {
			panic();
		}
		if (++fpga_power_domain_tree_desc[i + 2] >=
		    FPGA_MAX_CPUS_PER_CLUSTER) {
			panic();
		}

		/*
		 * Find out the number of clusters on the system by walking
		 * through the array of clusters and assuming that they are not
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

		fpga_update_affinity_boundaries();
	}

	return fpga_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id, thread_id;

	/* Ensure that the power domain tree is updated */
	(void)plat_get_power_domain_tree_desc();

	mpidr &= (MPIDR_AFFINITY_MASK | MPIDR_MT_MASK);
	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) {
		return -1;
	}

	if (mpidr & MPIDR_MT_MASK) {
		thread_id = MPIDR_AFFLVL0_VAL(mpidr);
	} else {
		thread_id = 0;
	}

	cpu_id = MPIDR_AFFLVL1_VAL(mpidr);
	cluster_id = MPIDR_AFFLVL2_VAL(mpidr);

	if (cluster_id >= fpga_power_domain_tree_desc[1]) {
		return -1;
	} else if (cpu_id > fpga_max_cpu_affvl) {
		return -1;
	} else if (thread_id > fpga_max_pe_affvl) {
		return -1;
	}

	/*
	 * The image running on the FPGA may or may not implement multithreading,
	 * and it shouldn't be assumed this is consistent across all CPUs.
	 * This ensures that any passed mpidr values reflect the status of the
	 * primary CPU's MT bit.
	 */
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);

	/* Calculate the correct core, catering for multi-threaded images */
	return (int) plat_fpga_calc_core_pos(mpidr);
}
