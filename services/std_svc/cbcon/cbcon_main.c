/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/coreboot.h>
#include <lib/smccc.h>
#include <services/cbcon_svc.h>
#include <smccc_helpers.h>

/* Check if this call is part of CBCON */
bool is_cbcon_fid(uint32_t smc_fid)
{
	return ((smc_fid == ARM_CBCON_VERSION) ||
		(smc_fid == ARM_CBCON_FEATURES) ||
		(smc_fid == ARM_CBCON_GET_ADDR));
}

uintptr_t cbcon_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			   u_register_t x3, u_register_t x4, void *cookie,
			   void *handle, u_register_t flags)
{
	switch (smc_fid) {
	case ARM_CBCON_VERSION:
		SMC_RET1(handle, MAKE_SMCCC_VERSION(
			CBCON_VERSION_MAJOR, CBCON_VERSION_MINOR));

	case ARM_CBCON_FEATURES:
		if (is_cbcon_fid((uint32_t)x1)) {
			SMC_RET1(handle, CBCON_E_SUCCESS);
		} else {
			SMC_RET1(handle, CBCON_E_NOT_SUPPORTED);
		}

	case ARM_CBCON_GET_ADDR:
		SMC_RET2(handle, CBCON_E_SUCCESS, (uint64_t)get_cbmem_console_base());

	default:
		WARN("Unimplemented CBCON Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, CBCON_E_NOT_IMPLEMENTED);
		break; /* unreachable */
	}
}
