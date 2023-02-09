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
	return (smc_fid == ARM_CBCON_GET_ADDR);
}

uintptr_t cbcon_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			    u_register_t x3, u_register_t x4, void *cookie,
			    void *handle, u_register_t flags)
{
	switch (smc_fid) {
	case ARM_CBCON_GET_ADDR:
		SMC_RET2(handle, CBCON_E_SUCCESS,
			 (uint64_t)coreboot_get_cbmem_console_base());

	default:
		WARN("Unimplemented CBCON Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, CBCON_E_NOT_IMPLEMENTED);
		break; /* unreachable */
	}
}
