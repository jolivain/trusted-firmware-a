/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_N2_H
#define NEOVERSE_N2_H

/* Cortex Neoverse N2 ID register for revision 0 */
#define NEOVERSE_N2_MIDR			U(0x410FD490)

/*******************************************************************************
 * CPU Activity Monitor Unit register specific definitions.
 ******************************************************************************/
#define NEOVERSE_N2_CPTR_TAM_BIT		(ULL(1) << 30)

/*******************************************************************************
 * CPU Power control register
 ******************************************************************************/
#define NEOVERSE_N2_CPUPWRCTLR_EL1		S3_0_C15_C2_7
#define NEOVERSE_N2_CORE_PWRDN_EN_BIT		(ULL(1) << 0)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_N2_CPUACTLR2_EL1		S3_0_C15_C1_1
#define NEOVERSE_N2_CPUACTLR2_EL1_BIT_2		(ULL(1) << 2)

#endif /* NEOVERSE_N2_H */
