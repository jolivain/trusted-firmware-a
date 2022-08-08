/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTSPMC_PRIVATE_H
#define MTSPMC_PRIVATE_H

#include <mtspmc_regs.h>

/* SPMC related registers */
#define SPM_MCUSYS_PWR_CON		SPM_REG(0x200)
#define SPM_MP0_CPUTOP_PWR_CON		SPM_REG(0x204)
#define SPM_MP0_CPU0_PWR_CON		SPM_REG(0x208)
#define SPM_MP0_CPU1_PWR_CON		SPM_REG(0x20c)
#define SPM_MP0_CPU2_PWR_CON		SPM_REG(0x210)
#define SPM_MP0_CPU3_PWR_CON		SPM_REG(0x214)
#define SPM_MP0_CPU4_PWR_CON		SPM_REG(0x218)
#define SPM_MP0_CPU5_PWR_CON		SPM_REG(0x21c)
#define SPM_MP0_CPU6_PWR_CON		SPM_REG(0x220)
#define SPM_MP0_CPU7_PWR_CON		SPM_REG(0x224)

/* per_cpu registers for SPM_MP0_CPU_PWR_CON */
static const struct per_cpu_reg SPM_CPU_PWR[] = {
	{ .cluster_addr = SPM_MP0_CPU0_PWR_CON, .cpu_stride = 2U }
};

/* per_cluster registers for SPM_MP0_CPUTOP_PWR_CON */
static const struct per_cpu_reg SPM_CLUSTER_PWR[] = {
	{ .cluster_addr = SPM_MP0_CPUTOP_PWR_CON, .cpu_stride = 0U }
};

/* per_cpu registers for MCUCFG_MP0_CLUSTER_CFG */
static const struct per_cpu_reg MCUCFG_BOOTADDR[] = {
	{ .cluster_addr = MCUCFG_MP0_CLUSTER_CFG8, .cpu_stride = 3U }
};

/* per_cpu registers for MCUCFG_MP0_CLUSTER_CFG5 */
static const struct per_cpu_reg MCUCFG_INITARCH[] = {
	{ .cluster_addr = MCUCFG_MP0_CLUSTER_CFG5, .cpu_stride = 0U }
};


static const struct per_cpu_reg MCUCFG_SPARK[] = {
	{ .cluster_addr = PTP3_CPU0_SPMC_SW_CFG, .cpu_stride = 11U }
};

static const struct per_cpu_reg ILDO_CONTROL5[] = {
	{ .cluster_addr = CPU0_ILDO_CONTROL5, .cpu_stride = 11U }
};

static const struct per_cpu_reg ILDO_CONTROL8[] = {
	{ .cluster_addr = CPU0_ILDO_CONTROL8, .cpu_stride = 11U }
};

#endif /* MTSPMC_PRIVATE_H */
