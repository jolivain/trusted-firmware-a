/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>

#include <mtk_sip_svc.h>
#include <plat_sip_calls.h>

static uint64_t mt_sip_pwr_on_mtcmos(uint32_t val)
{
	return MTK_SIP_E_SUCCESS;
}

static uint64_t mt_sip_pwr_off_mtcmos(uint32_t val)
{
	return MTK_SIP_E_SUCCESS;
}

uint64_t mediatek_plat_sip_handler(uint32_t smc_fid,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	uint64_t ret;

	switch (smc_fid) {
	case MTK_SIP_PWR_ON_MTCMOS:
		ret = mt_sip_pwr_on_mtcmos((uint32_t)x1);
		SMC_RET1(handle, ret);
	case MTK_SIP_PWR_OFF_MTCMOS:
		ret = mt_sip_pwr_off_mtcmos((uint32_t)x1);
		SMC_RET1(handle, ret);
	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}
