/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Dispatch synchronous system register traps from lower ELs.
 */

#include <context.h>
#include <bl31/sync_handle.h>

int handle_sysreg_trap(uint64_t esr_el3, cpu_context_t *ctx)
{
	switch (esr_el3 & ISS_SYSREG_OPCODE_MASK) {
	default:
		return -1;
	}
}
