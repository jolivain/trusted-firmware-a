/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/pauth_demangle.h>

#ifdef __aarch64__

/*
 * Strip the Pointer Authentication Code (PAC) from the address to retrieve the
 * original one.
 *
 * The PAC field is stored on the high bits of the address and defined as:
 * - PAC field = Xn[54:bottom_PAC_bit], when address tagging is used.
 * - PAC field = Xn[63:56, 54:bottom_PAC_bit], without address tagging.
 *
 * With bottom_PAC_bit = 64 - TCR_ELx.TnSZ
 */
uintptr_t demangle_address(uintptr_t addr, unsigned int el)
{
	unsigned int t0sz, bottom_pac_bit;
	uint64_t sctlr, tcr, pac_mask;

	/*
	 * Check if pointer authentication is enabled at the specified EL.
	 * If it isn't, we can then skip stripping a PAC code.
	 */
	if (el == 3U) {
		sctlr = read_sctlr_el3();
	} else if (el == 2U) {
		sctlr = read_sctlr_el2();
	} else {
		sctlr = read_sctlr_el1();
	}
	if ((sctlr & (SCTLR_EnIA_BIT | SCTLR_EnIB_BIT)) == 0) {
		return addr;
	}

	/*
	 * Different virtual address space size can be defined for each EL.
	 * Ensure that we use the proper one by reading the corresponding
	 * TCR_ELx register.
	 */
	if (el == 3U) {
		tcr = read_tcr_el3();
	} else if (el == 2U) {
		tcr = read_tcr_el2();
	} else {
		tcr = read_tcr_el1();
	}

	/* T0SZ = TCR_ELx[5:0] */
	t0sz = tcr & 0x1f;
	bottom_pac_bit = 64 - t0sz;
	pac_mask = (1ULL << bottom_pac_bit) - 1;

	/* demangle the address with the computed mask */
	return (addr & pac_mask);
}

#endif
