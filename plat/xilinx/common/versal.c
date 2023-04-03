/*
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <plat_startup.h>
#include <services/arm_arch_svc.h>

#include <pm_api_sys.h>
#include <pm_defs.h>

#define PMC_IDCODE_OFFSET               U(0x0)
#define PMC_VERSION_OFFSET              U(0x4)

#define PS_VERSION_MASK                 GENMASK(15, 8)
#define PS_VERSION_SHIFT                8U

/*
 * Array to get the chip idcode and chip version.
 * idcode = chip_info[0]
 * version = chip_info[1]
 */
static uint32_t chip_info[2];

int32_t plat_is_smccc_feature_available(u_register_t fid)
{
	switch (fid) {
	case SMCCC_ARCH_SOC_ID:
		return SMC_ARCH_CALL_SUCCESS;
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}

int32_t plat_get_soc_version(void)
{
	uint32_t ver, manfid;

	if (pm_get_chipid(chip_info) != PM_RET_SUCCESS) {
		NOTICE("Failed to read SOC information\n");
		return SMC_ARCH_CALL_INVAL_PARAM;
	}

	ver = (((chip_info[1]) & PS_VERSION_MASK) >> PS_VERSION_SHIFT);
	manfid = SOC_ID_SET_JEP_106(JEDEC_XILINX_BKID, JEDEC_XILINX_MFID);

	return (int32_t)(manfid | (ver & SOC_ID_IMPL_DEF_MASK));
}

int32_t plat_get_soc_revision(void)
{
	/* The pm_get_chip() returns both soc version and soc revision.
	 * Return the soc revision got from the pm_get_chip() called from
	 * plat_get_soc_version().
	 */
	if (chip_info[0] == 0) {
		return SMC_ARCH_CALL_INVAL_PARAM;
	}

	return chip_info[0];
}
