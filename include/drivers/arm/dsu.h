/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DSU_H
#define DSU_H

#define PMCR_N_MAX			0x1f

#define read_cluster_eventctr_num()	((read_clusterpmcr() >> CLUSTERPMCR_N_SHIFT) \
		& CLUSTERPMCR_N_MASK)

#define SAVE_PMU_REG(state, reg) state->reg = read_##reg()

#define RESTORE_PMU_REG(reg, value) write_##reg(value)

typedef struct cluster_pmu_state{
	uint64_t clusterpmcr;
	uint64_t clusterpmcntenset;
	uint64_t clusterpmccntr;
	uint64_t clusterpmovsset;
	uint64_t clusterpmselr;
	uint64_t clusterpmsevtyper;
	uint64_t counter_val[PMCR_N_MAX];
	uint64_t counter_type[PMCR_N_MAX];
} cluster_pmu_state_t;

extern cluster_pmu_state_t cluster_pmu_context;

void dsu_save_state(cluster_pmu_state_t *cluster_pmu_context);

void dsu_restore_state(cluster_pmu_state_t *cluster_pmu_context);

#endif /* DSU_H */
