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
#include <mtk_sip_inc.h>
/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
#define DEVINFO_SIZE 4
#define LINUX_KERNEL_32 0
#define LINUX_KERNEL_64 1
#define SMC32_PARAM_MASK		(0xFFFFFFFF)

struct atf_arg_t {
	uint32_t atf_magic;
	uint32_t tee_support;
	uint64_t tee_entry;
	uint64_t tee_boot_arg_addr;
	uint32_t hwuid[4];      /* HW Unique id for t-base used */
	uint32_t HRID[8];       /* HW random id for t-base used */
	uint32_t devinfo[DEVINFO_SIZE];
};

#define MTK_BL31_BOOT_ARG(_args, _member) ((struct mtk_bl31_fw_config *)_args)->_member

struct mtk_bl31_fw_config {
	void *from_bl2; /* MTK boot tag */
	void *soc_fw_config;
	void *hw_config;
	void *reserved;
};

enum {
	BOOT_ARG_FROM_BL2,
	BOOT_ARG_SOC_FW_CONFIG,
	BOOT_ARG_HW_CONFIG,
	BOOT_ARG_RESERVED
};

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
	uint64_t atf_arg_addr;
};

/* Declarations for mtk_plat_common.c */
uint32_t plat_get_spsr_for_bl32_entry(void);
uint32_t plat_get_spsr_for_bl33_entry(void);
void clean_top_32b_of_param(uint32_t smc_fid, u_register_t *x1,
				u_register_t *x2,
				u_register_t *x3,
				u_register_t *x4);
void bl31_prepare_kernel_entry(uint64_t k32_64);
void enable_ns_access_to_cpuectlr(void);
uint64_t get_kernel_info_pc(void);
uint64_t get_kernel_info_r0(void);
uint64_t get_kernel_info_r1(void);
uint64_t get_kernel_info_r2(void);
void *get_mtk_bl31_fw_config(int index);
void populate_bl33_image_ep(entry_point_info_t *bl33_ep_instance,
			struct mtk_bl_param_t *p_mtk_bl_param);
u_register_t boot_to_kernel(u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *handle,
	struct smccc_res *ret);

extern struct atf_arg_t gatfarg;
#endif
#endif /* MTK_COMMON_H */

