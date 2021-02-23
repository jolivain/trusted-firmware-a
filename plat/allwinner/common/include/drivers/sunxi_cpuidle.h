/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_CPUIDLE_H
#define SUNXI_CPUIDLE_H

#include <lib/psci/psci.h>

#if SUNXI_HAVE_CPUIDLE

void sunxi_cpuidle_on_finish(void);
void sunxi_cpuidle_off(void);
void sunxi_cpuidle_suspend(const psci_power_state_t *target_state);
void sunxi_cpuidle_suspend_finish(const psci_power_state_t *target_state);
void sunxi_cpuidle_get_sys_suspend_power_state(psci_power_state_t *req_state);
int sunxi_cpuidle_validate_power_state(unsigned int power_state,
				       psci_power_state_t *req_state);

void sunxi_cpuidle_init(void);

#else

static inline void sunxi_cpuidle_on_finish(void) {}
static inline void sunxi_cpuidle_off(void) {}
static inline void sunxi_cpuidle_suspend(const psci_power_state_t *target_state) {}
static inline void sunxi_cpuidle_suspend_finish(const psci_power_state_t *target_state) {}
static inline void sunxi_cpuidle_get_sys_suspend_power_state(psci_power_state_t *req_state) {}
static inline int sunxi_cpuidle_validate_power_state(unsigned int power_state,
						     psci_power_state_t *req_state)
{
	return PSCI_E_INVALID_PARAMS;
}

static inline void sunxi_cpuidle_init(void) {}

#define sunxi_cpuidle_suspend NULL
#define sunxi_cpuidle_suspend_finish NULL
#define sunxi_cpuidle_get_sys_suspend_power_state NULL
#define sunxi_cpuidle_validate_power_state NULL

#endif

#endif /* SUNXI_CPUIDLE_H */
