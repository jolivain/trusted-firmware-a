/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/css/dsu.h>

#include <plat/common/platform.h>

/****************************************************************************
 * This function, save_dsu_pmu_state, is designed to save the
 * current state of the Performance Monitoring Unit (PMU) for a cluster.
 *
 * The function performs the following operations:
 * 1. Saves the current values of several PMU registers
 *    (CLUSTERPMCR_EL1, CLUSTERPMCNTENSET_EL1, CLUSTERPMCCNTR_EL1,
 *    CLUSTERPMOVSSET_EL1, and CLUSTERPMSELR_EL1) into the cluster_pmu_state i
 *    structure.
 *
 * 2. Disables the PMU event counting by
 *    clearing the E bit in the clusterpmcr_el1 register.
 *
 * 3. Iterates over the available PMU counters as
 *    determined by the read_cluster_eventctr_num() function.
 *    For each counter, it:
 *    a. Selects the counter by writing its index to CLUSTERPMSELR_EL1.
 *    b. Reads the current counter value (event count) and
 *       the event type being counted from CLUSTERPMXEVCNTR_EL1 and
 *       CLUSTERPMXEVTYPER_EL1 registers, respectively.
 *
 * This function is useful for preserving the DynamIQ Shared Unit's (DSU)
 * PMU registers over a power cycle.
 *
 ***************************************************************************/

#if PRESERVE_DSU_PMU_REGS
void save_dsu_pmu_state(cluster_pmu_state_t *cluster_pmu_state)
{

	unsigned int idx = 0U;
	unsigned int cluster_eventctr_num = read_cluster_eventctr_num();

	assert(cluster_pmu_state != 0);

	SAVE_PMU_REG(cluster_pmu_state, clusterpmcr);

	write_clusterpmcr(cluster_pmu_state->clusterpmcr &
			~(CLUSTERPMCR_E_BIT));

	SAVE_PMU_REG(cluster_pmu_state, clusterpmcntenset);

	SAVE_PMU_REG(cluster_pmu_state, clusterpmccntr);

	SAVE_PMU_REG(cluster_pmu_state, clusterpmovsset);

	SAVE_PMU_REG(cluster_pmu_state, clusterpmselr);

	for (idx = 0U ; idx < cluster_eventctr_num ; idx++) {
		write_clusterpmselr(idx);
		cluster_pmu_state->counter_val[idx] = read_clusterpmxevcntr();
		cluster_pmu_state->counter_type[idx] = read_clusterpmxevtyper();
	}
}

void cluster_off_dsu_pmu_context_save(cluster_pmu_state_t (*cluster_ctx)[])
{
	unsigned int cluster_pos;

	cluster_pos = (unsigned int)
		(plat_my_core_pos() / ((plat_get_power_domain_tree_desc())[1]));

	save_dsu_pmu_state(&(*cluster_ctx)[cluster_pos]);
}

/*****************************************************************************
 * This function, restore_dsu_pmu_state, restores the state of the
 * Performance Monitoring Unit (PMU) from a previously saved state.
 *
 * The function performs the following operations:
 * 1. Restores the CLUSTERPMCR_EL1 register with the
 *    saved value from the cluster_pmu_state structure.
 * 2. Iterates over the available PMU counters as determined
 *    by the read_cluster_eventctr_num() function. For each counter, it:
 *    a. Selects the counter by writing its index to CLUSTERPMSELR_EL1.
 *    b. Restores the counter value (event count) and the event type to
 *       CLUSTERPMXEVCNTR_EL1 and CLUSTERPMXEVTYPER_EL1 registers, respectively
 * 3. Restores several other PMU registers (CLUSTERPMSELR_EL1,
 *    CLUSTERPMOVSCLR_EL1, CLUSTERPMOVSSET_EL1, CLUSTERPMCCNTR_EL1,
 *    and CLUSTERPMCNTENSET_EL1) with their saved values.
 *
 *****************************************************************************/

void restore_dsu_pmu_state(cluster_pmu_state_t *cluster_pmu_state)
{
	unsigned int idx = 0U;
	unsigned int cluster_eventctr_num = read_cluster_eventctr_num();

	assert(cluster_pmu_state != 0);

	for (idx = 0U ; idx < cluster_eventctr_num ; idx++) {
		write_clusterpmselr(idx);
		RESTORE_PMU_REG(clusterpmxevcntr, cluster_pmu_state->counter_val[idx]);
		RESTORE_PMU_REG(clusterpmxevtyper, cluster_pmu_state->counter_type[idx]);
	}

	RESTORE_PMU_REG(clusterpmselr, cluster_pmu_state->clusterpmselr);

	RESTORE_PMU_REG(clusterpmovsclr, ~(uint32_t)cluster_pmu_state->clusterpmovsset);

	RESTORE_PMU_REG(clusterpmovsset, cluster_pmu_state->clusterpmovsset);

	RESTORE_PMU_REG(clusterpmccntr, cluster_pmu_state->clusterpmccntr);

	RESTORE_PMU_REG(clusterpmcntenset, cluster_pmu_state->clusterpmcntenset);

	write_clusterpmcr(cluster_pmu_state->clusterpmcr);
}

void cluster_on_dsu_pmu_context_restore(cluster_pmu_state_t (*cluster_ctx)[])
{
	int core_pos;
	unsigned char cluster_pos;

	core_pos = plat_my_core_pos();

	cluster_pos = (unsigned char)
		(core_pos / ((plat_get_power_domain_tree_desc())[1]));

	restore_dsu_pmu_state(&(*cluster_ctx)[cluster_pos]);
}

#endif
