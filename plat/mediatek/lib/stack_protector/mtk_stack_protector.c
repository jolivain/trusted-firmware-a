/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <plat/common/platform.h>

#define DEFAULT_CANARY_VALUE ((u_register_t) 3288484550995823360ULL)

u_register_t plat_get_stack_protector_canary(void)
{
	/*
	 * Return a new value to be used for the stack protection's canary.
	 *
	 * Ideally, this value is a random number that is impossible to predict by an
	 * attacker.
	 */
	return DEFAULT_CANARY_VALUE ^ read_cntpct_el0();
}

