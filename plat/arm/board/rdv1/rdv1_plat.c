/*
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat/common/platform.h>
#include <smccc_helpers.h>

#include <sgi_plat.h>

unsigned int plat_arm_sgi_get_platform_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_ID_OFFSET)
				& SID_SYSTEM_ID_PART_NUM_MASK;
}

unsigned int plat_arm_sgi_get_config_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_CFG_OFFSET);
}

unsigned int plat_arm_sgi_get_multi_chip_mode(void)
{
	return (mmio_read_32(SID_REG_BASE + SID_NODE_ID_OFFSET) &
			SID_MULTI_CHIP_MODE_MASK) >> SID_MULTI_CHIP_MODE_SHIFT;
}

void bl31_platform_setup(void)
{
	sgi_bl31_common_platform_setup();
}

/* SiP handler specific to each Arm platform. */
uintptr_t arm_plat_sip_handler(uint32_t smc_fid,
				u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *cookie,
				void *handle,
				u_register_t flags)
{
	WARN("Unimplemented ARM SiP Service Call: 0x%x \n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}
