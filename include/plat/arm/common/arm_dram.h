/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_DRAM_H
#define ARM_DRAM__H

#include <stdint.h>

/* Number of DRAM banks supported */
#define DRAM_BANKS_NUM	2

struct dram_bank {
	uintptr_t base;	/* base address */
	size_t size;	/* size of bank */
};

#endif /* ARM_DRAM_H */
