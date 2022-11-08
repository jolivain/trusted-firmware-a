/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <arch_features.h>
#include <services/trng_svc.h>
#include <plat/common/platform.h>

#define RANDOM_CANARY_VALUE ((u_register_t) 3288484550995823360ULL)

bool plat_get_entropy(uint64_t *out)
{
	/* Use the RNDR instruction if the CPU supports it */
	if (is_feat_rng_supported()) {
		*out = read_rndr();
		return true;
	}

	/*
	 * Ideally, a random number should be returned above. If a random
	 * number generator is not supported, return instead a
	 * combination of a timer's value and a compile-time constant.
	 * This is better than nothing but not necessarily really secure.
	 */
	*out = RANDOM_CANARY_VALUE ^ read_cntpct_el0();
	return true;
}

DEFINE_SVC_UUID2(qemu_trng_uuid,
		 0xaa767875, 0xe700, 0x4d66, 0xbf, 0xb0,
		 0xdb, 0x35, 0xb4, 0xdf, 0x73, 0x25
);
uuid_t plat_trng_uuid;

void plat_entropy_setup(void)
{
	plat_trng_uuid = qemu_trng_uuid;
}
