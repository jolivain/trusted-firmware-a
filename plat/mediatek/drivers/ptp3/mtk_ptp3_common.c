/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved. \
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <lib/pm/mtk_pm.h>
#include <mtk_ptp3_common.h>
#include <mtk_ptp3_main.h>

void ptp3_core_init(unsigned int core)
{
	ptp3_init(core);
	pdp_init(core);
	dt_init(core);
}

void ptp3_core_deinit(unsigned int core)
{
	/* TBD */
}

#if MTK_PUBEVENT_ENABLE
/* Handle for power on domain */
void *ptp3_handle_pwr_on_event(const void *arg)
{
	if (arg != NULL) {
		struct mt_cpupm_event_data *data = (struct mt_cpupm_event_data *)arg;
		if ((data->pwr_domain & MT_CPUPM_PWR_DOMAIN_CORE) > 0) {
			ptp3_core_init(data->cpuid);
		}
	}
	return (void *)arg;
}
MT_CPUPM_SUBCRIBE_EVENT_PWR_ON(ptp3_handle_pwr_on_event);

/* Handle for power off domain */
void *ptp3_handle_pwr_off_event(const void *arg)
{
	if (arg != NULL) {
		struct mt_cpupm_event_data *data = (struct mt_cpupm_event_data *)arg;
		if ((data->pwr_domain & MT_CPUPM_PWR_DOMAIN_CORE) > 0) {
			ptp3_core_deinit(data->cpuid);
		}
	}
	return (void *)arg;
}
MT_CPUPM_SUBCRIBE_EVENT_PWR_OFF(ptp3_handle_pwr_off_event);
#else
#pragma message "PSCI hint not enable"
#endif
