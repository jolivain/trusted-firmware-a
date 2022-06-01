/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_SIP_INC_H
#define MTK_SIP_INC_H

#include <lib/smccc.h>
#if CONFIG_MTK_GZ
#include <mtk_gz_api.h>
#endif
#include <mtk_sip_def.h>

extern uint32_t el1_is_2nd_bootloader;
/*
 * These macros below are used to identify SIP calls from Kernel,
 * Hypervisor, or 2ndBootloader
 */
#define SIP_FID_ORI_MASK		0xc000
#define SIP_FID_ORI_SHIFT		14
#define SIP_FID_KERNEL			0x0
#define SIP_FID_KERNEL_VIA_GZ	0x1
#define SIP_FID_GZ				0x2

#define GET_SMC_ORI(_fid) \
	(((_fid) & SIP_FID_ORI_MASK) >> SIP_FID_ORI_SHIFT)
#define GET_SMC_ORI_NUM(_fid) \
	((_fid) & ~(SIP_FID_ORI_MASK))

#define is_from_nsel2(_ori) \
	(_ori == SIP_FID_GZ)
#define is_from_bl33(_ori) \
	((_ori != SIP_FID_GZ) && (el1_is_2nd_bootloader == 1))
#define is_from_nsel1(_ori) \
	(((_ori == SIP_FID_KERNEL) || (_ori == SIP_FID_KERNEL_VIA_GZ)) \
		&& (el1_is_2nd_bootloader == 0))

#ifdef CONFIG_MTK_GZ
#define is_smc_forbidden(_ori) \
	(is_el2_enabled() && _ori == SIP_FID_KERNEL)
#else
#define is_smc_forbidden(_ori) \
	(_ori == SIP_FID_KERNEL_VIA_GZ)
#endif

struct smccc_res {
	uint64_t a1;
	uint64_t a2;
	uint64_t a3;
};
typedef u_register_t (*smc_handler_t)(u_register_t,
									u_register_t,
									u_register_t,
									u_register_t,
									void *,
									struct smccc_res *);
struct smc_descriptor {
	smc_handler_t smc_handler;
	const uint32_t smc_id_aarch32;
	const uint32_t smc_id_aarch64;
	const char *smc_name;
	short *const smc_descriptor_index;
};
#define MASK_32_BIT 0xffffffff
#define MTK_SIP_FID(smc_cc, func_num)			\
		((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |	\
		 ((smc_cc) << FUNCID_CC_SHIFT) |	\
		 (OEN_SIP_START << FUNCID_OEN_SHIFT) |	\
		 ((func_num) << FUNCID_NUM_SHIFT))

#define SMC_ID_EXPAND_AS_ENUM(_smc_id, _smc_num) \
	_smc_id##_AARCH32 = ((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |	\
		 ((0) << FUNCID_CC_SHIFT) |	\
		 (OEN_SIP_START << FUNCID_OEN_SHIFT) |	\
		 ((_smc_num) << FUNCID_NUM_SHIFT)), \
	_smc_id##_AARCH64 = ((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |	\
		 ((1) << FUNCID_CC_SHIFT) |	\
		 (OEN_SIP_START << FUNCID_OEN_SHIFT) |	\
		 ((_smc_num) << FUNCID_NUM_SHIFT)),

#define SMC_ID_EXPAND_AS_DESCRIPTOR_INDEX(_smc_id, _smc_num) \
short _smc_id##_descriptor_index __section("mtk_plat_ro") = -1;

#define SMC_ID_EXPAND_AS_EXTERN_SMC_INDEX(_smc_id, _smc_num) \
extern short _smc_id##_descriptor_index;

#define SMC_ID_EXPAND_AS_FUNC_EXTERN(_smc_id, _smc_handler) \
u_register_t _smc_handler(u_register_t x1, \
			u_register_t x2, u_register_t x3, \
			u_register_t x4, void *handle);

#define DECLARE_SMC_HANDLER(_smc_id, _smc_handler) \
const struct smc_descriptor _smc_id##_descriptor \
	__used \
	__aligned(sizeof(void *)) \
	__section(".mtk_smc_descriptor_pool") \
	= { \
		.smc_handler = _smc_handler, \
		.smc_name = #_smc_id, \
		.smc_id_aarch32 = _smc_id##_AARCH32, \
		.smc_id_aarch64 = _smc_id##_AARCH64, \
		.smc_descriptor_index = &_smc_id##_descriptor_index \
	};

enum {
	MTK_SIP_SMC_FROM_BL33_TABLE(SMC_ID_EXPAND_AS_ENUM)
	MTK_SIP_SMC_FROM_NS_EL1_TABLE(SMC_ID_EXPAND_AS_ENUM)
	MTK_SIP_SMC_FROM_S_EL1_TABLE(SMC_ID_EXPAND_AS_ENUM)
	MTK_SIP_SMC_FROM_NS_EL2_TABLE(SMC_ID_EXPAND_AS_ENUM)
	MTK_SIP_SMC_MAX_NUMBER
};
int mtk_smc_handler_init(void);
/* Expand SiP SMC table */
MTK_SIP_SMC_FROM_BL33_TABLE(SMC_ID_EXPAND_AS_EXTERN_SMC_INDEX);
MTK_SIP_SMC_FROM_NS_EL1_TABLE(SMC_ID_EXPAND_AS_EXTERN_SMC_INDEX);
MTK_SIP_SMC_FROM_S_EL1_TABLE(SMC_ID_EXPAND_AS_EXTERN_SMC_INDEX);
MTK_SIP_SMC_FROM_NS_EL2_TABLE(SMC_ID_EXPAND_AS_EXTERN_SMC_INDEX);

#endif /* MTK_SIP_INC_H */
