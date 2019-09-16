/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <stdint.h>
#include <string.h>

#include "aml_private.h"

static int aml_sip_get_chip_id(uint64_t version)
{
	void *dst = (void *)AML_SHARE_MEM_OUTPUT_BASE;
	uint8_t id[16];
	uint32_t size;
	int ret;

	size = (version == 2) ? 16 : 12;

	if ((ret = aml_scpi_get_chip_id(id, &size)) != 0)
		return ret;

	if (version == 2 && size == 16) {
		*((uint32_t *)dst) = 2;
		memcpy(dst + 4, id, sizeof(id));
	} else {
		/*
		 * struct aml_cpu_info {
		 *      unsigned int version;
		 *      uint8_t chipid[12];
		 *      unsigned int reserved[103];
		 * }
		 */
		memset(dst, 0, 428);
		*((uint32_t *)dst) = 1;
		memcpy(dst + 4, id, 12);
	}

	return 0;
}

/*******************************************************************************
 * This function is responsible for handling all SiP calls
 ******************************************************************************/
static uintptr_t aml_sip_handler(uint32_t smc_fid,
				  u_register_t x1, u_register_t x2,
				  u_register_t x3, u_register_t x4,
				  void *cookie, void *handle,
				  u_register_t flags)
{
	switch (smc_fid) {

	case AML_SM_GET_SHARE_MEM_INPUT_BASE:
		SMC_RET1(handle, AML_SHARE_MEM_INPUT_BASE);

	case AML_SM_GET_SHARE_MEM_OUTPUT_BASE:
		SMC_RET1(handle, AML_SHARE_MEM_OUTPUT_BASE);

	case AML_SM_EFUSE_READ:
	{
		void *dst = (void *)AML_SHARE_MEM_OUTPUT_BASE;
		uint64_t ret = aml_efuse_read(dst, (uint32_t)x1, x2);

		SMC_RET1(handle, ret);
	}
	case AML_SM_EFUSE_USER_MAX:
		SMC_RET1(handle,  aml_efuse_user_max());

	case AML_SM_JTAG_ON:
		aml_scpi_jtag_set_state(AML_JTAG_STATE_ON, x1);
		SMC_RET1(handle, 0);

	case AML_SM_JTAG_OFF:
		aml_scpi_jtag_set_state(AML_JTAG_STATE_OFF, x1);
		SMC_RET1(handle, 0);

	case AML_SM_GET_CHIP_ID:
		SMC_RET1(handle, aml_sip_get_chip_id(x1));

	default:
		ERROR("BL31: Unhandled SIP SMC: 0x%08x\n", smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}

DECLARE_RT_SVC(
	aml_sip_handler,

	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	aml_sip_handler
);
