/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 * Copyright (c) 2021-2022, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A78_AE_H
#define CORTEX_A78_AE_H

#include <cortex_a78.h>

#define CORTEX_A78_AE_MIDR U(0x410FD420)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A78_AE_CPUECTLR_EL1			CORTEX_A78_CPUECTLR_EL1
#define CORTEX_A78_AE_CPUECTLR_EL1_BIT_8		CORTEX_A78_CPUECTLR_EL1_BIT_8

/*******************************************************************************
 * CPU Auxiliary Control register 2 specific definitions.
 ******************************************************************************/
#define CORTEX_A78_AE_ACTLR2_EL1			CORTEX_A78_ACTLR2_EL1
#define CORTEX_A78_AE_ACTLR2_EL1_BIT_0			CORTEX_A78_ACTLR2_EL1_BIT_0
#define CORTEX_A78_AE_ACTLR2_EL1_BIT_40			CORTEX_A78_ACTLR2_EL1_BIT_40

#endif /* CORTEX_A78_AE_H */
