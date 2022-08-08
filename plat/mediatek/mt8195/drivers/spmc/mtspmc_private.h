/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTSPMC_PRIVATE_H
#define MTSPMC_PRIVATE_H

#include <mtspmc_regs.h>

/* SPMC related registers */
#define SPM_MCUSYS_PWR_CON		MCUCFG_REG(0xd200)
#define SPM_MP0_CPUTOP_PWR_CON		MCUCFG_REG(0xd204)
#define SPM_MP0_CPU0_PWR_CON		MCUCFG_REG(0xd208)
#define SPM_MP0_CPU1_PWR_CON		MCUCFG_REG(0xd20c)
#define SPM_MP0_CPU2_PWR_CON		MCUCFG_REG(0xd210)
#define SPM_MP0_CPU3_PWR_CON		MCUCFG_REG(0xd214)
#define SPM_MP0_CPU4_PWR_CON		MCUCFG_REG(0xd218)
#define SPM_MP0_CPU5_PWR_CON		MCUCFG_REG(0xd21c)
#define SPM_MP0_CPU6_PWR_CON		MCUCFG_REG(0xd220)
#define SPM_MP0_CPU7_PWR_CON		MCUCFG_REG(0xd224)

/* per_cpu registers for SPM_MP0_CPU_PWR_CON */
static const struct per_cpu_reg SPM_CPU_PWR[] = {
	{ .cluster_addr = SPM_MP0_CPU0_PWR_CON, .cpu_stride = 2U }
};

/* per_cluster registers for SPM_MP0_CPUTOP_PWR_CON */
static const struct per_cpu_reg SPM_CLUSTER_PWR[] = {
	{ .cluster_addr = SPM_MP0_CPUTOP_PWR_CON, .cpu_stride = 0U }
};

/* MCUSYS DREQ BIG VPROC ISO control */
#define DREQ20_BIG_VPROC_ISO		MCUCFG_REG(0xad8c)

/* per_cpu registers for MCUCFG_MP0_CLUSTER_CFG */
static const struct per_cpu_reg MCUCFG_BOOTADDR[] = {
	{ .cluster_addr = MCUCFG_MP0_CLUSTER_CFG8, .cpu_stride = 3U }
};

/* per_cpu registers for MCUCFG_MP0_CLUSTER_CFG5 */
static const struct per_cpu_reg MCUCFG_INITARCH[] = {
	{ .cluster_addr = MCUCFG_MP0_CLUSTER_CFG5, .cpu_stride = 0U }
};

/* bit-fields of CPC_FLOW_CTRL_CFG */
#define SSPM_CORE_PWR_ON_EN		BIT(7) /* for cpu-hotplug */

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
