/*
 * Copyright (c) 2020, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_COMMON_H
#define MTK_COMMON_H

#include <common/bl_common.h>
#include <common/param_header.h>
#include <plat/common/common_def.h>

/* TF-A mandatory platform constants */
#define MAP_BL31_TOTAL		MAP_REGION_FLAT(			\
					BL31_START,			\
					BL31_END - BL31_START,		\
					MT_MEMORY | MT_RW | MT_SECURE)

#define TZRAM_BASE		MTK_BL31_BASE
#define TZRAM_SIZE		MTK_BL31_MAX_SIZE
/*
 * BL31_BASE is calculated using the current BL3-1 debug size plus a
 * little space for growth.
 */
#define BL31_BASE		(TZRAM_BASE)
#define BL31_LIMIT		(TZRAM_BASE + TZRAM_SIZE - BL31_RESERVED_MEM_SIZE - BUFFER_FOR_TEE)


#if SEPARATE_CODE_AND_RODATA
#define MAP_BL_RO		\
					MAP_REGION_FLAT(			\
						BL_CODE_BASE,			\
						BL_CODE_END - BL_CODE_BASE,	\
						MT_CODE | MT_SECURE),		\
						MAP_REGION_FLAT(			\
						BL_RO_DATA_BASE,		\
						BL_RO_DATA_END			\
							- BL_RO_DATA_BASE,	\
						MT_RO_DATA | MT_SECURE), \
					MAP_REGION_FLAT(			\
						MTK_RODATA_START,		\
						MTK_RODATA_END	\
							- MTK_RODATA_START,	\
						MT_RO_DATA | MT_SECURE)
#else
#define MAP_BL_RO			MAP_REGION_FLAT(			\
						BL_CODE_BASE,			\
						BL_CODE_END - BL_CODE_BASE,	\
						MT_CODE | MT_SECURE)
#endif
#ifndef __ASSEMBLER__
#include <stdint.h>
#include <mtk_sip_svc.h>
/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
#define DEVINFO_SIZE 4
#define LINUX_KERNEL_32 0
#define LINUX_KERNEL_64 1
#define SMC32_PARAM_MASK		(0xFFFFFFFF)

/* Declarations for mtk_plat_common.c */
uint32_t plat_get_spsr_for_bl33_entry(void);
void clean_top_32b_of_param(uint32_t smc_fid, u_register_t *x1,
				u_register_t *x2,
				u_register_t *x3,
				u_register_t *x4);
void *get_mtk_bl31_fw_config(int index);
u_register_t boot_to_kernel(u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *handle,
	struct smccc_res *ret);
#endif
#endif /* MTK_COMMON_H */
