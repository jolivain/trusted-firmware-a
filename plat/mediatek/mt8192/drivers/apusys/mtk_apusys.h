/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTK_APUSYS_H__
#define __MTK_APUSYS_H__

#include <stdint.h>

/* setup the SMC command ops */
#define MTK_SIP_APU_START_MCU	(0x00)
#define MTK_SIP_APU_STOP_MCU	(0x01)

/* AO Register */
#define AO_MD32_PRE_DEFINE        (APUSYS_APU_S_S_4_BASE + 0x00)
#define AO_MD32_BOOT_CTRL         (APUSYS_APU_S_S_4_BASE + 0x04)
#define AO_MD32_SYS_CTRL          (APUSYS_APU_S_S_4_BASE + 0x08)
#define AO_SEC_FW                 (APUSYS_APU_S_S_4_BASE + 0x10)

#define PRE_DEFINE_CACHE_TCM	(0x3)
#define	PRE_DEFINE_CACHE	(0x2)
#define PRE_DEFINE_SHIFT_0G	(0)
#define PRE_DEFINE_SHIFT_1G	(2)
#define PRE_DEFINE_SHIFT_2G	(4)
#define PRE_DEFINE_SHIFT_3G	(6)

#define SEC_FW_NON_SECURE	(1)
#define SEC_FW_SHIFT_NS		(4)
#define SEC_FW_DOMAIN_SHIFT	(0)

#define SYS_CTRL_RUN		(0)
#define SYS_CTRL_STALL		(1)

/* Reviser Register */
#define REVISER_SECUREFW_CTXT     (APUSYS_SCTRL_REVISER_BASE + 0x300)
#define REVISER_USDRFW_CTXT       (APUSYS_SCTRL_REVISER_BASE + 0x304)

int32_t apusys_kernel_ctrl(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4,
			   uint32_t *ret1);
#endif /* __MTK_APUSYS_H__ */
