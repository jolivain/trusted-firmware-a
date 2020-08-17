/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <plat/common/platform.h>

#include "psci_private.h"

#ifndef PLAT_CORES_PWRDWN_WAIT_TIMEOUT_MS
#define PLAT_CORES_PWRDWN_WAIT_TIMEOUT_MS 1000
#endif

#if IMAGE_BL31
void psci_stop_other_cores(void)
{
	int idx, this_cpu_idx, cnt;

	this_cpu_idx = plat_my_core_pos();

	/* Raise G0 IPI cpustop to all cores but self */
	for (idx = 0; idx < psci_plat_core_count; idx++) {
		if ((idx != this_cpu_idx) &&
		    (psci_get_aff_info_state_by_idx(idx) == AFF_STATE_ON)) {
			plat_ipi_send_cpu_stop(psci_cpu_pd_nodes[idx].mpidr);
		}
	}

	 /* Wait for others cores to shutdown */
	for (cnt = 0; cnt < PLAT_CORES_PWRDWN_WAIT_TIMEOUT_MS; cnt++) {
		if (psci_is_last_on_cpu())
			break;
		mdelay(1);
	}

	if (!psci_is_last_on_cpu()) {
		WARN("Failed to stop all cores!\n");
		psci_print_power_domain_map();
	}
}
#endif

void __dead2 psci_system_off(void)
{
	psci_print_power_domain_map();

	assert(psci_plat_pm_ops->system_off != NULL);

	/* Notify the Secure Payload Dispatcher */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_system_off != NULL)) {
		psci_spd_pm->svc_system_off();
	}

	(void) console_flush();

	/* Call the platform specific hook */
	psci_plat_pm_ops->system_off();

	/* This function does not return. We should never get here */
}

void __dead2 psci_system_reset(void)
{
	psci_print_power_domain_map();

	assert(psci_plat_pm_ops->system_reset != NULL);

	/* Notify the Secure Payload Dispatcher */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_system_reset != NULL)) {
		psci_spd_pm->svc_system_reset();
	}

	(void) console_flush();

	/* Call the platform specific hook */
	psci_plat_pm_ops->system_reset();

	/* This function does not return. We should never get here */
}

u_register_t psci_system_reset2(uint32_t reset_type, u_register_t cookie)
{
	unsigned int is_vendor;

	psci_print_power_domain_map();

	assert(psci_plat_pm_ops->system_reset2 != NULL);

	is_vendor = (reset_type >> PSCI_RESET2_TYPE_VENDOR_SHIFT) & 1U;
	if (is_vendor == 0U) {
		/*
		 * Only WARM_RESET is allowed for architectural type resets.
		 */
		if (reset_type != PSCI_RESET2_SYSTEM_WARM_RESET)
			return (u_register_t) PSCI_E_INVALID_PARAMS;
		if ((psci_plat_pm_ops->write_mem_protect != NULL) &&
		    (psci_plat_pm_ops->write_mem_protect(0) < 0)) {
			return (u_register_t) PSCI_E_NOT_SUPPORTED;
		}
	}

	/* Notify the Secure Payload Dispatcher */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_system_reset != NULL)) {
		psci_spd_pm->svc_system_reset();
	}
	(void) console_flush();

	return (u_register_t)
		psci_plat_pm_ops->system_reset2((int) is_vendor, reset_type,
						cookie);
}
