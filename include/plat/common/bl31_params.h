/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef COMMON_BL31_PLAT_PARAMS_H
#define COMMON_BL31_PLAT_PARAMS_H

#include <stdbool.h>
#include <stdint.h>

#include "bl31_params_raw.h"

/* Helper to extract entry points from bl_params structures (version 1 and 2) */
void bl31_params_parse(u_register_t param, entry_point_info_t *bl32_ep_info_out,
		       entry_point_info_t *bl33_ep_info_out);

/* Handler function that handles an individual platform parameter. Returns true
 * if the parameter was recognized and false if not. */
typedef bool (*bl31_plat_param_handler_t)(bl31_plat_param_header_t *param);

/* Interprets plat_param as the start of a platform parameter list, and passes
 * the parameters individually to handler(). Handles generic parameters directly
 * if handler() hasn't already done so. If only generic parameters are expected,
 * handler() can be NULL. */
void bl31_plat_params_parse(u_register_t plat_param,
			    bl31_plat_param_handler_t handler);

#endif /* COMMON_BL31_PLAT_PARAMS_H */
