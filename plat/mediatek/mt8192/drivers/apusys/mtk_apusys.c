/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include "mtk_apusys.h"

int32_t apusys_kernel_ctrl(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4,
							uint32_t *ret1)
{
	uint32_t request_ops;
	int32_t ret = 0;

	request_ops = (uint32_t)x1;
	INFO("[APUSYS] ops=0x%x\n", request_ops);

	switch (request_ops) {
	case MTK_SIP_APU_IOMMU_INIT:
		mmio_write_32(AO_SEC_FW, (uint32_t)x2);
		break;
	case MTK_SIP_APU_BOOT_CTRL:
		mmio_write_32(AO_MD32_BOOT_CTRL, (uint32_t)x2);
		mmio_write_32(AO_MD32_PRE_DEFINE, 0xAB);
		break;
	case MTK_SIP_APU_SYS_CTRL:
		mmio_write_32(AO_MD32_SYS_CTRL, (uint32_t)x2);
		break;
	default:
		ERROR("%s, unknown request_ops = %x\n", __func__, request_ops);
		ret = -1;
		break;
	}

	return ret;
}
