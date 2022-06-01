/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MTK_PLAT_COMMON_H
#define MTK_PLAT_COMMON_H

#include <stdint.h>

#include <common/bl_common.h>
#include <common/param_header.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
#define DEVINFO_SIZE 4
#define LINUX_KERNEL_32 0
#define SMC32_PARAM_MASK		(0xFFFFFFFF)

#define JEDEC_MTK_BKID U(4)
#define JEDEC_MTK_MFID U(0x26)

struct kernel_info {
	uint64_t pc;
	uint64_t r0;
	uint64_t r1;
	uint64_t r2;
	uint64_t k32_64;
};

struct mtk_bl_param_t {
	uint64_t bootarg_loc;
	uint64_t bootarg_size;
	uint64_t bl33_start_addr;
	uint64_t tee_info_addr;
};

struct mtk_bl31_params {
       param_header_t h;
       image_info_t *bl31_image_info;
       entry_point_info_t *bl32_ep_info;
       image_info_t *bl32_image_info;
       entry_point_info_t *bl33_ep_info;
       image_info_t *bl33_image_info;
};

/* Declarations for mtk_plat_common.c */
void clean_top_32b_of_param(uint32_t smc_fid, u_register_t *x1,
				u_register_t *x2,
				u_register_t *x3,
				u_register_t *x4);

#endif /* MTK_PLAT_COMMON_H */
