/*
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AGILEX5_DDR_H
#define AGILEX5_DDR_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <lib/utils_def.h>

#include "socfpga_handoff.h"

#define CONFIG_NR_DRAM_BANKS	1

typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;
typedef phys_addr_t fdt_addr_t;

/* DDR/RAM configuration */
typedef struct ddr_info_t {
	phys_addr_t start;
	phys_size_t size;
} ddr_info;

int agilex5_ddr_init(handoff *hoff_ptr);

#endif /* AGILEX5_DDR_H */
