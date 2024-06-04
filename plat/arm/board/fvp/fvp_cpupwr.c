/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if __aarch64__

#include <arch_helpers.h>
#include <drivers/arm/fvp/fvp_cpupwr.h>
#include <lib/utils_def.h>

#include <aem_generic.h>
#include <cortex_a35.h>
#include <cortex_a53.h>
#include <cortex_a57.h>
#include <cortex_a72.h>
#include <cortex_a73.h>
#include <cortex_a78_ae.h>
#include <neoverse_e1.h>

bool check_cpupwrctrl_el1_is_available(void)
{
	/* Poupulate list of CPU midr that doesnt support CPUPWRCTL_EL1 */
	unsigned int midr_no_cpupwrctl[] = {
		BASE_AEM_MIDR,
		CORTEX_A35_MIDR,
		CORTEX_A53_MIDR,
		CORTEX_A57_MIDR,
		CORTEX_A72_MIDR,
		CORTEX_A73_MIDR,
		CORTEX_A78_AE_MIDR,
		NEOVERSE_E1_MIDR,
	};

	unsigned int i = 0, midr = 0;
	midr = (unsigned int)read_midr();
	bool ret = true;

	for (i = 0; i < ARRAY_SIZE(midr_no_cpupwrctl); i++)
	{
		if (midr_no_cpupwrctl[i] == midr)
		{
			ret = false;
		}
	}

	return ret;
}

#endif /* __arch64__ */
