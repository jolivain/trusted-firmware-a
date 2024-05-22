/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/el3_runtime/pubsub.h>

void debug_init_el3(void)
{
	u_register_t val;

	/* When FEAT_Debugv8p9 is implemented:
	 *
	 * MDCR_EL3.EBWE: Set to 0b1
	 * Enables use of additional breakpoints or watchpoints,
	 * and enables a trap to EL3 on accesses to debug registers
	 */

	val = read_mdcr_el3();
	val |= MDCR_EBWE_BIT;
	write_mdcr_el3(val);
}
