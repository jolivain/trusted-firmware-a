/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTK_APUSYS_H__
#define __MTK_APUSYS_H__

#include <stdint.h>

/* setup the SMC command ops */
#define MTK_SIP_APU_IOMMU_INIT	(0x00)
#define MTK_SIP_APU_BOOT_CTRL	(0x01)
#define MTK_SIP_APU_SYS_CTRL	(0x02)

/* AO Register */
#define AO_MD32_PRE_DEFINE        (APUSYS_APU_S_S_4_BASE + 0x00)
#define AO_MD32_BOOT_CTRL         (APUSYS_APU_S_S_4_BASE + 0x04)
#define AO_MD32_SYS_CTRL          (APUSYS_APU_S_S_4_BASE + 0x08)
#define AO_SEC_FW                 (APUSYS_APU_S_S_4_BASE + 0x10)

/* Reviser Register */
#define REVISER_SECUREFW_CTXT     (APUSYS_SCTRL_REVISER_BASE + 0x300)
#define REVISER_USDRFW_CTXT       (APUSYS_SCTRL_REVISER_BASE + 0x304)

int32_t apusys_kernel_ctrl(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4,
			   uint32_t *ret1);
#endif /* __MTK_APUSYS_H__ */
