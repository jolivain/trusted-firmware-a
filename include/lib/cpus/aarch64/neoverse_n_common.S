/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/******************************************************************************
 * Neoverse Nx CPU Configuration register definitions
 *****************************************************************************/
#define CPUCFR_EL1		S3_0_C15_C0_0

/* SCU bit of CPU Configuration Register, EL1 */
#define SCU_SHIFT		U(2)

/*
 * Check if the SCU L3 Unit is present on the DSU
 * 1-> SCU present
 * 0-> SCU not present
 *
 * This function is implemented as weak on dsu_helpers.S and must be
 * overwritten for Neoverse Nx cores.
 */

func is_scu_present_in_dsu
	mrs	x0, CPUCFR_EL1
	ubfx	x0, x0, #SCU_SHIFT, #1
	eor	x0, x0, #1
	ret
endfunc is_scu_present_in_dsu
