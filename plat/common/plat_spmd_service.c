/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <smccc_helpers.h>
#include <services/ffa_svc.h>
#include <services/spmd_plat.h>

int spmd_handle_platform_message(unsigned long long msg,
	unsigned long long parm1, unsigned long long parm2,
	unsigned long long parm3, unsigned long long parm4,
	void *handle)
{
	VERBOSE("%s %llx %llx %llx %llx %llx\n", __func__,
		msg, parm1, parm2, parm3, parm4);

	SMC_RET8(handle, FFA_SUCCESS_SMC32,
		 FFA_TARGET_INFO_MBZ, 0,
		 msg,
		 parm1, parm2, parm3, parm4);
}
