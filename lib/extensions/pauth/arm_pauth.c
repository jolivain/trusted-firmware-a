/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <assert.h>
#include <cdefs.h>
#include <stdint.h>

/*
 * Instruction pointer authentication key A. The low 64-bit are at [0], and the
 * high bits at [1].
 */
uint64_t plat_apiakey[2];

/*******************************************************************************
 * Handle all possible cases regarding ARMv8.3-PAuth.
 ******************************************************************************/
void bl_init_pauth(void)
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
	 *
	 * Obtain 128-bit instruction key A from the platform and save it to the
	 * system registers. Pointer authentication can't be enabled here or the
	 * authentication will fail when returning from this function.
	 */
	assert(is_armv8_3_pauth_apa_api_present());

	/* This is only a toy implementation to generate a seemingly random
	 * 128-bit key from sp and x30 values. A production system must
	 * re-implement this function to generate keys from a reliable
	 * randomness source.
	 */
	uintptr_t return_addr = (uintptr_t)__builtin_return_address(0U);
	uintptr_t frame_addr = (uintptr_t)__builtin_frame_address(0U);

	plat_apiakey[0] = (return_addr << 13) ^ frame_addr;
	plat_apiakey[1] = (frame_addr << 15) ^ return_addr;

	write_apiakeylo_el1(plat_apiakey[0]);
	write_apiakeyhi_el1(plat_apiakey[1]);
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
