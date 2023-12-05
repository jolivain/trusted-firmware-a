/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/psci/psci.h>

static const plat_psci_ops_t plat_plat_pm_ops = {};

int plat_setup_psci_ops(uintptr_t secure_entrypoint,
			const plat_psci_ops_t **plat_ops)
{
	*plat_ops = &plat_plat_pm_ops;

	return 0;
}
