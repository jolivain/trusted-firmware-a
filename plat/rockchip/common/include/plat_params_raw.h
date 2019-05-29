/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_PLAT_ROCKCHIP_COMMON_PLAT_PARAMS_RAW_H
#define ARM_TRUSTED_FIRMWARE_PLAT_ROCKCHIP_COMMON_PLAT_PARAMS_RAW_H

/**************** NOTE ************** NOTE ************** NOTE ****************
 * This file is intended to be included by third-party projects. It should    *
 * not contain any #include directives or function prototypes. It should only *
 * contain constant and type definitions, only rely on ISO C fixed-width      *
 * integer types and not depend on the definition of any macros other than    *
 * U(), AARCH32/64 and __ASSEMBLY__. It should never be included directly,    *
 * but only through the corresponding wrapper header, which must include      *
 * relevant headers (like <stdint.h>) before this file.                       *
 **************** NOTE ************** NOTE ************** NOTE ****************/

/* param type */
enum bl_aux_rk_param_type {
	BL_AUX_PARAM_RK_RESET_GPIO = BL_AUX_PARAM_VENDOR_SPECIFIC_FIRST,
	BL_AUX_PARAM_RK_POWEROFF_GPIO,
	BL_AUX_PARAM_RK_SUSPEND_GPIO,
	BL_AUX_PARAM_RK_SUSPEND_APIO,
};

typedef struct {
	uint8_t apio1 : 1;
	uint8_t apio2 : 1;
	uint8_t apio3 : 1;
	uint8_t apio4 : 1;
	uint8_t apio5 : 1;
} bl_aux_rk_apio_info_t;

typedef struct {
	bl_aux_param_header_t h;
	bl_aux_rk_apio_info_t apio;
} bl_aux_param_rk_apio_t;

#endif /* ARM_TRUSTED_FIRMWARE_PLAT_ROCKCHIP_COMMON_PLAT_PARAMS_RAW_H */
