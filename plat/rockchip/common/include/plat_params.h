/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PARAMS_H
#define PLAT_PARAMS_H

#include <plat/common/bl31_params.h>
#include <stdint.h>

/* param type */
enum bl31_rk_param_type {
	BL31_PLAT_PARAM_RK_RESET_GPIO = BL31_PLAT_PARAM_VENDOR_SPECIFIC_FIRST,
	BL31_PLAT_PARAM_RK_POWEROFF_GPIO,
	BL31_PLAT_PARAM_RK_SUSPEND_GPIO,
	BL31_PLAT_PARAM_RK_SUSPEND_APIO,
};

typedef struct {
	uint8_t apio1 : 1;
	uint8_t apio2 : 1;
	uint8_t apio3 : 1;
	uint8_t apio4 : 1;
	uint8_t apio5 : 1;
} bl31_rk_apio_info_t;

typedef struct {
	bl31_plat_param_header_t h;
	bl31_rk_apio_info_t apio;
} bl31_plat_param_rk_apio_t;

#endif /* PLAT_PARAMS_H */
