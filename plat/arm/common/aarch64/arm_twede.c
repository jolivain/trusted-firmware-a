/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#pragma weak plat_arm_set_twedel_scr_el3
#pragma weak plat_arm_set_twedel_hcr_el2
#pragma weak plat_arm_set_twedel_sctlr_elx

/*******************************************************************************
 * In v8.6+ platforms with delayed trapping of WFE this hook sets the delay. It
 * is a weak function definition so can be overridden depending on the
 * requirements of a platform.  These default functions are weakly defined and
 * return zero so this feature is disabled unless they are replaced by platform-
 * specific functions returning non-zero values.
 ******************************************************************************/

uint32_t plat_arm_set_twedel_scr_el3 (void)
{
	return 0;
}

uint32_t plat_arm_set_twedel_hcr_el2 (void)
{
	return 0;
}

uint32_t plat_arm_set_twedel_sctlr_elx (void)
{
	return 0;
}
