/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX7_DEF_H__
#define __IMX7_DEF_H__

#include <stdint.h>


/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void plat_imx7_io_setup(void);
void imx7_platform_setup(u_register_t arg1, u_register_t arg2,
			 u_register_t arg3, u_register_t arg4);

#endif /*__IMX7_DEF_H__ */
