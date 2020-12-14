/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_SPM_CONSTRAINT_H__
#define __MT_SPM_CONSTRAINT_H__

#include <mt_lp_rm.h>

#define MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF	(1U << 0)
#define MT_RM_CONSTRAINT_ALLOW_DRAM_S0		(1U << 1)
#define MT_RM_CONSTRAINT_ALLOW_DRAM_S1		(1U << 2)
#define MT_RM_CONSTRAINT_ALLOW_VCORE_LP		(1U << 3)
#define MT_RM_CONSTRAINT_ALLOW_INFRA_PDN	(1U << 4)
#define MT_RM_CONSTRAINT_ALLOW_BUS26M_OFF	(1U << 5)
#define MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND	(1U << 6)
#define MT_RM_CONSTRAINT_ALLOW_BBLPM		(1U << 7)
#define MT_RM_CONSTRAINT_ALLOW_XO_UFS		(1U << 8)
#define MT_RM_CONSTRAINT_ALLOW_GPS_STATE	(1U << 9)
#define MT_RM_CONSTRAINT_ALLOW_LVTS_STATE	(1U << 10)

#define MT_SPM_RC_INVALID		0x0
#define MT_SPM_RC_VALID_SW		(1U << 0)
#define MT_SPM_RC_VALID_FW		(1U << 1)
#define MT_SPM_RC_VALID_RESIDNECY	(1U << 2)
#define MT_SPM_RC_VALID_COND_CHECK	(1U << 3)
#define MT_SPM_RC_VALID_COND_LATCH	(1U << 4)
#define MT_SPM_RC_VALID_UFS_H8		(1U << 5)
#define MT_SPM_RC_VALID_FLIGHTMODE	(1U << 6)
#define MT_SPM_RC_VALID_XSOC_BBLPM	(1U << 7)
#define MT_SPM_RC_VALID_TRACE_EVENT	(1U << 8)

#define MT_SPM_RC_VALID	(MT_SPM_RC_VALID_SW | MT_SPM_RC_VALID_FW)

#define IS_MT_RM_RC_READY(status)	\
	((status & MT_SPM_RC_VALID) == MT_SPM_RC_VALID)

#define MT_SPM_RC_BBLPM_MODE		\
	(MT_SPM_RC_VALID_UFS_H8 |	\
	 MT_SPM_RC_VALID_FLIGHTMODE |	\
	 MT_SPM_RC_VALID_XSOC_BBLPM)

#define IS_MT_SPM_RC_BBLPM_MODE(st)	\
	((st & (MT_SPM_RC_BBLPM_MODE)) == MT_SPM_RC_BBLPM_MODE)

#define MT_SPM_RC_EXTERN_STATUS_SET(v, st) ({v |= (st & 0xffff); })
#define MT_SPM_RC_EXTERN_STATUS_CLR(v, st) ({v &= ~(st & 0xffff); })

struct constraint_status {
	uint16_t id;
	uint16_t is_valid;
	uint32_t is_cond_block;
	uint32_t enter_cnt;
	unsigned long long residency;
	struct mt_spm_cond_tables *cond_res;
};

enum MT_SPM_RM_RC_TYPE {
	MT_RM_CONSTRAINT_ID_BUS26M,
	MT_RM_CONSTRAINT_ID_SYSPLL,
	MT_RM_CONSTRAINT_ID_DRAM,
	MT_RM_CONSTRAINT_ID_CPU_BUCK_LDO,
	MT_RM_CONSTRAINT_ID_ALL,
};

enum CONSTRAINT_STATUS_UPDATE_TYPE {
	CONSTRAINT_UPDATE_VALID,
	CONSTRAINT_UPDATE_COND_CHECK,
	CONSTRAINT_RESIDNECY,
};

enum CONSTRAINT_STATUS_GET_TYPE {
	CONSTRAINT_GET_VALID = 0xD0000000,
	CONSTRAINT_GET_ENTER_CNT,
	CONSTRAINT_GET_RESIDENCY,
	CONSTRAINT_GET_COND_EN,
	CONSTRAINT_COND_BLOCK,
	CONSTRAINT_GET_COND_BLOCK_LATCH,
	CONSTRAINT_GET_COND_BLOCK_DETAIL,
	CONSTRAINT_GET_RESIDNECY,
};

struct rc_common_state {
	unsigned int id;
	unsigned int act;
	unsigned int type;
	void *value;
};
#endif /* __MT_SPM_CONSTRAINT_H__ */
