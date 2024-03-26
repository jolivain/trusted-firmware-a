/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_PRIVATE_H
#define FVP_PRIVATE_H

#include <plat/arm/common/plat_arm.h>


#define FVP_RMM_CONSOLE_BASE		UL(0x1c0c0000)
#define FVP_RMM_CONSOLE_BAUD		UL(115200)
#define FVP_RMM_CONSOLE_CLK_IN_HZ	UL(14745600)
#define FVP_RMM_CONSOLE_NAME		"pl011"

#define FVP_RMM_CONSOLE_COUNT		UL(1)

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/

void fvp_config_setup(void);

void fvp_interconnect_init(void);
void fvp_interconnect_enable(void);
void fvp_interconnect_disable(void);
void fvp_timer_init(void);
void tsp_early_platform_setup(void);

#endif /* FVP_PRIVATE_H */
