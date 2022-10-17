/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_PTP3_MAIN_H
#define MTK_PTP3_MAIN_H

#include <lib/mmio.h>
#include <lib/utils_def.h>

/* CPU Info */
#define NR_PTP3_CFG_CPU			U(8)
#define PTP3_CFG_CPU_START_ID_L		U(0)
#define PTP3_CFG_CPU_START_ID_B		U(6)
#define PTP3_CFG_CPU_END_ID		U(7)

#define NR_PTP3_CFG1_DATA		U(2)
#define PTP3_CFG1_MASK			(0x3000)

#define NR_PTP3_CFG2_DATA		U(5)

#define PTP3_CFG3_MASK1			(0x1180)
#define PTP3_CFG3_MASK2			(0x35C0)
#define PTP3_CFG3_MASK3			(0x3DC0)

#endif /* MTK_PTP3_MAIN_H */
