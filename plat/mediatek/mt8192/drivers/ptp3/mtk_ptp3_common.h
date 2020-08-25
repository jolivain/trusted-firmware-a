/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_PTP3_H
#define MTK_PTP3_H

#include <lib/mmio.h>

/************************************************
 * BIT Operation and REG r/w
 ************************************************/
#define ptp3_read(addr) mmio_read_32((uintptr_t)addr)
#define ptp3_write(addr,  val) mmio_write_32((uintptr_t)addr, val)

/************************************************
 * CPU info
 ************************************************/
#define NR_PTP3_CFG1_CPU 8
#define PTP3_CFG1_CPU_START_ID 0
#define PTP3_CFG1_CPU_END_ID 7
#define NR_PTP3_CFG1_DATA 2
#define PTP3_CFG1_MASK 0x00100000

#define NR_PTP3_CFG2_CPU 4
#define PTP3_CFG2_CPU_START_ID 4
#define PTP3_CFG2_CPU_END_ID 7
#define NR_PTP3_CFG2_DATA 2

#define NR_PTP3_CFG3_CPU 4
#define PTP3_CFG3_CPU_START_ID 4
#define PTP3_CFG3_CPU_END_ID 7
#define NR_PTP3_CFG3_DATA 2

/************************************************
 * config enum
 ************************************************/
enum PTP3_CFG {
	PTP3_CFG_ADDR,
	PTP3_CFG_VALUE,

	NR_PTP3_CFG,
};

/************************************
 * prototype
 ************************************/
/* init trigger for ptp3 feature */
extern void ptp3_init(unsigned int core);
extern void ptp3_deinit(unsigned int core);

#endif /* MTK_PTP3_H */
