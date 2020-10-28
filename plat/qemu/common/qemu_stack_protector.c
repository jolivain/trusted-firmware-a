/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <arch_features.h>
#include <plat/common/platform.h>

#define RANDOM_CANARY_VALUE ((u_register_t) 3288484550995823360ULL)

u_register_t plat_get_stack_protector_canary(void)
{
#if ENABLE_FEAT_RNG
	/* Use the RNDR instruction if the CPU supports it */
	if (is_armv8_5_rng_present()) {
		return read_rndr();
	}
#endif
	
	/*
	 * Ideally, a random number should be returned instead of the
	 * combination of a timer's value and a compile-time constant.
	 * As the virt platform does not have any random number generator,
	 * this is better than nothing but not necessarily really secure.
	 */
	return RANDOM_CANARY_VALUE ^ read_cntpct_el0();
}

