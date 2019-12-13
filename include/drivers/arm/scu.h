/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBSA_H
#define SBSA_H

#include <stdint.h>

#define SCU_BASE_REG	0x1C000000
#define SCU_CTRL_REG	0x00
#define SCU_CFG_REG	0x04

#define SCU_ENABLE_BIT	(1 << 0)

typedef struct scu_conf_desc {
	unsigned int num_cpus:2;
	unsigned int cpu_smp:4;
	unsigned int cpu0_data_cache_size:4;
	unsigned int cpu1_data_cache_size:4;
	unsigned int cpu2_data_cache_size:4;
	unsigned int cpu3_data_cache_size:4;
	/* accelerator coherrency port */
	unsigned int acp_present:1;
	/* Indicates the presence of a 
	 * PL310 L2 cache controller connected 
	 * to the processor */
	unsigned int l2_cache_ctrl_present:1;
} scu_conf_desc_t;

void enable_snoop_ctrl_unit(uintptr_t base);
scu_conf_desc_t *config_snoop_ctrl_unit(uintptr_t base);

#endif /* SBSA_H */
