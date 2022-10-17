/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_PTP3_COMMON_H
#define MTK_PTP3_COMMON_H

#include <lib/mmio.h>
#include <lib/utils_def.h>

/* register read/write */
#define ptp3_write(addr, val) mmio_write_32((uintptr_t)addr, val)
#define ptp3_clrsetbits(addr, clear, set) mmio_clrsetbits_32((uintptr_t)addr, clear, set)

/* config enum */
enum PTP3_CFG {
	PTP3_CFG_ADDR,
	PTP3_CFG_VALUE,
	NR_PTP3_CFG,
};

/* prototype */
void ptp3_init(unsigned int core);
void pdp_init(unsigned int pdp_cpu);
void dt_init(unsigned int dt_cpu);

#endif /* MTK_PTP3_COMMON_H */
