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

unsigned char fpga_core_bitmap[CORE_BITMAP_SIZE];
unsigned char mt_bitmap[(FPGA_MAX_CLUSTER_COUNT * FPGA_MAX_CPUS_PER_CLUSTER) \
									/ 8];
unsigned char fpga_power_domain_tree_desc[PLATFORM_CORE_COUNT + 2];
uint32_t fpga_mpids[PLATFORM_CORE_COUNT];

spinlock_t topology_tree_spinlock;

static void fpga_generate_power_topology_tree(void)
{
	/*
	 * Iterates over fpga_core_bitmap to generate the power domain tree
	 * description. At the same time, it populates an array to map any
	 * valid MPIDR with its CPUID.
	 */

	unsigned int i, j, mpids_idx;
	int max_cluster = -1;
	unsigned int afflv0, afflv1, afflv2;
	uint32_t mpidr;
	unsigned int threads[FPGA_MAX_CLUSTER_COUNT + \
				FPGA_MAX_CPUS_PER_CLUSTER] = {0U};
	void *src;
	void *dst;

	mpids_idx = 0;

	for (i = 0U; i < CORE_BITMAP_SIZE; i++) {
		for (j = 0U; j < 8; j++) {
			unsigned int count;
			unsigned int core_idx;
			unsigned int pe_pos = i * 8 + j;

			if (!(fpga_core_bitmap[i] & (1 << j))) {
				continue;
			}

			if (mpids_idx == PLATFORM_CORE_COUNT) {
				panic();
			}

			afflv0 = pe_pos & ((1 << FPGA_MAX_PE_PER_CPU_SHIFT) - 1);
			afflv1 = pe_pos >> CORE_MAP_AFF1_SHIFT;
			afflv1 &= ((1 << FPGA_MAX_CPUS_PER_CLUSTER_SHIFT) - 1);
			afflv2 = pe_pos >> CORE_MAP_AFF2_SHIFT;
			afflv2 &= ((1 << FPGA_MAX_PE_PER_CPU_SHIFT) - 1);

			/* Update the count of cores on the cluster */
			fpga_power_domain_tree_desc[2 + afflv2]++;
			if (fpga_power_domain_tree_desc[2 + afflv2] >
						FPGA_MAX_CPUS_PER_CLUSTER) {
				panic();
			}

			/*
			 * Update the count of threads for the current core
			 * assuming that a single thread core will have a
			 * count of one thread.
			 */
			count = ++threads[afflv2 * FPGA_MAX_CPUS_PER_CLUSTER +
								afflv1];
			if (count > FPGA_MAX_PE_PER_CPU) {
				panic();
			}

			/* Update the clusters count */
			if ((int)afflv2 > max_cluster) {
				max_cluster = afflv2;
				fpga_power_domain_tree_desc[1]++;
				if (fpga_power_domain_tree_desc[1] >
					FPGA_MAX_CLUSTER_COUNT) {
					panic();
				}
			}

			/* Update the list of available MPIDRS */
			mpidr = afflv0 & MPIDR_AFFLVL_MASK;
			mpidr |= (afflv1 & MPIDR_AFFLVL_MASK) <<
							MPIDR_AFF1_SHIFT;
			mpidr |= (afflv2 & MPIDR_AFFLVL_MASK) <<
							MPIDR_AFF2_SHIFT;

			core_idx = (afflv1 & FPGA_MAX_CPUS_PER_CLUSTER_SHIFT) |
				   (afflv2 & FPGA_MAX_CLUSTER_COUNT_SHIFT)
					<< FPGA_MAX_CPUS_PER_CLUSTER_SHIFT;
			if (mt_bitmap[core_idx >> 3] & (1 << (core_idx & 0x7))) {
				mpidr |= MPIDR_MT_MASK;
			}
			fpga_mpids[mpids_idx++] = mpidr;
		}
	}

	if (mpids_idx < PLATFORM_CORE_COUNT) {
		fpga_mpids[mpids_idx] = INVALID_MPID;
	}

	/*
	 * Populate the rest of the topology tree whith the threads per
	 * core now that the total number of clusters is known.
	 */
	max_cluster = fpga_power_domain_tree_desc[1];
	j = 0;
	for (i = 0; i < max_cluster; i++) {
		src = (void *)&threads[FPGA_MAX_CPUS_PER_CLUSTER * i];
		dst = (void *)&fpga_power_domain_tree_desc[2 + max_cluster + j];

		memcpy(dst, src, fpga_power_domain_tree_desc[2 + i]);
		/* CPUs on this cluster */
		j = fpga_power_domain_tree_desc[2 + i];
	}
}

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	uint64_t mpidr;
	uint64_t bitmap_idx;

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
		 * Power Domain Topology Tree has not been initialized, so
		 * perform the initialization here.
		 */

		dsbsy(); /* Ensure all writes by other cores are commited. */

		mpidr = read_mpidr_el1() & MPID_MASK;

		/* Add this core to the core bitmap */
		bitmap_idx = (MPIDR_AFFLVL0_VAL(mpidr) & FPGA_MAX_PE_PER_CPU_SHIFT);
		bitmap_idx |= ((MPIDR_AFFLVL1_VAL(mpidr) &
				FPGA_MAX_CPUS_PER_CLUSTER_SHIFT) <<
							CORE_MAP_AFF1_SHIFT);
		bitmap_idx |= ((MPIDR_AFFLVL2_VAL(mpidr) &
				FPGA_MAX_CLUSTER_COUNT_SHIFT) <<
							CORE_MAP_AFF2_SHIFT);

		fpga_core_bitmap[bitmap_idx >> 3] |= (1 << (bitmap_idx & 0x7));

		/* Store the MT status for this core */
		if (mpidr & MPIDR_MT_MASK) {
			bitmap_idx = (MPIDR_AFFLVL1_VAL(mpidr) &
					FPGA_MAX_CPUS_PER_CLUSTER_SHIFT);
			bitmap_idx |= ((MPIDR_AFFLVL2_VAL(mpidr) &
					FPGA_MAX_CLUSTER_COUNT_SHIFT) <<
					FPGA_MAX_CPUS_PER_CLUSTER_SHIFT);

			mt_bitmap[bitmap_idx >> 3] |= (1 << (bitmap_idx & 0x7));
		}

		fpga_generate_power_topology_tree();

		fpga_power_domain_tree_desc[0] = 1;
	}

	spin_unlock(&topology_tree_spinlock);

	return fpga_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	/*
	 * Iterate over the list of valid MPIDRs and return the index where
	 * mpidr is stored or -1 if it cannot be found.
	 */
	unsigned int i;

	/*
	 * Check if the topology three has been initialized. If not, initialize
	 * it, as it is needed to get the valid MPIDR array
	 */
	if (fpga_power_domain_tree_desc[0] == 0) {
		(void)plat_get_power_domain_tree_desc();
	}

	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);
	mpidr &= MPID_MASK;

	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		if (mpidr == fpga_mpids[i]) {
			return (int)i;
		}
		if (fpga_mpids[i] == INVALID_MPID) {
			return -1;
		}
	}
	return -1;
}
