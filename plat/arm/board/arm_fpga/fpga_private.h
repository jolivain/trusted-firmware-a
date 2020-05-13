/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FPGA_PRIVATE_H
#define FPGA_PRIVATE_H

#include "../fpga_def.h"
#include <platform_def.h>

#define C_RUNTIME_READY_KEY (0xaa55aa55)

#define CORE_MAP_AFF0_SHIFT	0
#define CORE_MAP_AFF1_SHIFT	FPGA_MAX_PE_PER_CPU_SHIFT
#define CORE_MAP_AFF2_SHIFT	(CORE_MAP_AFF1_SHIFT + \
				 FPGA_MAX_CPUS_PER_CLUSTER_SHIFT)

#ifndef __ASSEMBLER__

#define CORE_BITMAP_SIZE ((1 <<					\
			  (FPGA_MAX_CLUSTER_COUNT_SHIFT +	\
			   FPGA_MAX_CPUS_PER_CLUSTER_SHIFT +	\
			   FPGA_MAX_PE_PER_CPU_SHIFT)) / 8)

extern unsigned char fpga_power_domain_tree_desc[PLATFORM_CORE_COUNT + 2];

void fpga_console_init(void);

void plat_fpga_gic_init(void);
void fpga_pwr_gic_on_finish(void);
void fpga_pwr_gic_off(void);
#endif /* __ASSEMBLER__ */

#endif /* FPGA_PRIVATE_H */
