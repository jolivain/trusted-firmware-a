/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <assert.h>
#include <cdefs.h>
#include <stdint.h>
#include <lib/el3_runtime/cpu_data.h>
#include <plat/common/platform.h>

/*******************************************************************************
 * Handle all possible cases regarding ARMv8.3-PAuth.
 ******************************************************************************/
void bl_check_pauth_present(void)
{
#if ENABLE_PAUTH
	/*
	 * ENABLE_PAUTH = 1 && CTX_INCLUDE_PAUTH_REGS = 1
	 *
	 * Check that the system supports address authentication to avoid
	 * getting an access fault when accessing the registers. This is all
	 * that is needed to check. If any of the authentication mechanisms is
	 * supported, the system knows about ARMv8.3-PAuth, so all the registers
	 * are available and accessing them won't generate a fault.
	 */
	assert(is_armv8_3_pauth_apa_api_present());
#else
	/*
	 * ENABLE_PAUTH = 0 && CTX_INCLUDE_PAUTH_REGS = 1
	 *
	 * Assert that the ARMv8.3-PAuth registers are present or an access
	 * fault will be triggered when they are being saved or restored.
	 */
	assert(is_armv8_3_pauth_present());
#endif /* ENABLE_PAUTH */
}
