/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_SIP_SVC_H
#define MTK_SIP_SVC_H

#include <stdint.h>
#include <lib/smccc.h>
#if CONFIG_MTK_GZ
#include <mtk_gz_api.h>
#endif
#include <mtk_sip_def.h>

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_CALL_COUNT		0x8200ff00
#define SIP_SVC_UID			0x8200ff01
/*					0x8200ff02 is reserved */
#define SIP_SVC_VERSION			0x8200ff03

/* Mediatek SiP Service Calls version numbers */
#define MTK_SIP_SVC_VERSION_MAJOR	0x0
#define MTK_SIP_SVC_VERSION_MINOR	0x1

#define SMC_AARCH64_BIT		0x40000000

/* Number of Mediatek SiP Calls implemented */
#define MTK_COMMON_SIP_NUM_CALLS	4

/* Mediatek SiP Service Calls function IDs */
#define MTK_SIP_SET_AUTHORIZED_SECURE_REG	0x82000001

#define SMC_ID_EXPAND_AS_ENUM(_smc_id, _smc_num) \
	_smc_id##_AARCH32 = ((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) | \
		 ((0) << FUNCID_CC_SHIFT) | \
		 (OEN_SIP_START << FUNCID_OEN_SHIFT) | \
		 ((_smc_num) << FUNCID_NUM_SHIFT)), \
	_smc_id##_AARCH64 = ((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) | \
		 ((1) << FUNCID_CC_SHIFT) | \
		 (OEN_SIP_START << FUNCID_OEN_SHIFT) | \
		 ((_smc_num) << FUNCID_NUM_SHIFT)),

#define SMC_ID_EXPAND_AS_EXTERN_SMC_INDEX(_smc_id, _smc_num) \
	extern short _smc_id##_descriptor_index;

#define SMC_ID_EXPAND_AS_FUNC_EXTERN(_smc_id, _smc_handler) \
	u_register_t _smc_handler(u_register_t x1, \
			u_register_t x2, u_register_t x3, \
			u_register_t x4, void *handle);

/* Bind SMC handler with SMC ID */
#define DECLARE_SMC_HANDLER(_smc_id, _smc_handler) \
	const struct smc_descriptor _smc_id##_descriptor \
		__used \
		__aligned(sizeof(void *)) \
		__section(".mtk_smc_descriptor_pool") = { \
			.smc_handler = _smc_handler, \
			.smc_name = #_smc_id, \
			.smc_id_aarch32 = _smc_id##_AARCH32, \
			.smc_id_aarch64 = _smc_id##_AARCH64, \
			.smc_descriptor_index = &_smc_id##_descriptor_index \
		};
MTK_SIP_SMC_FROM_BL33_TABLE(SMC_ID_EXPAND_AS_EXTERN_SMC_INDEX);
MTK_SIP_SMC_FROM_NS_EL1_TABLE(SMC_ID_EXPAND_AS_EXTERN_SMC_INDEX);

/* Expand SiP SMC ID table as enum */
enum {
	MTK_SIP_SMC_FROM_BL33_TABLE(SMC_ID_EXPAND_AS_ENUM)
	MTK_SIP_SMC_FROM_NS_EL1_TABLE(SMC_ID_EXPAND_AS_ENUM)
	MTK_SIP_SMC_MAX_NUMBER
};

/* Mediatek SiP Calls error code */
enum {
	MTK_SIP_E_SUCCESS = 0,
	MTK_SIP_E_INVALID_PARAM = -1,
	MTK_SIP_E_NOT_SUPPORTED = -2,
	MTK_SIP_E_INVALID_RANGE = -3,
	MTK_SIP_E_PERMISSION_DENY = -4,
	MTK_SIP_E_LOCK_FAIL = -5
};

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

/*
 * This function should be implemented in Mediatek SOC directory. It fullfills
 * MTK_SIP_SET_AUTHORIZED_SECURE_REG SiP call by checking the sreg with the
 * predefined secure register list, if a match was found, set val to sreg.
 *
 * Return MTK_SIP_E_SUCCESS on success, and MTK_SIP_E_INVALID_PARAM on failure.
 */
uint64_t mt_sip_set_authorized_sreg(uint32_t sreg, uint32_t val);

#endif /* MTK_SIP_SVC_H */
