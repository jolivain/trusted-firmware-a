/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <arch_helpers.h>
#include <drivers/el3_uptime.h>
#include <mtk_sip_svc.h>
#include <platform_def.h>

uint64_t align_time_base;
uint64_t el3_time_base;
unsigned int mt_log_ktime_sync;

void el3_uptime_init(uint64_t align_time_start, uint64_t el3_time_start)
{
	align_time_base += align_time_start;
	el3_time_base = el3_time_start;
}

uint64_t el3_uptime(void)
{
	uint64_t cval;

	cval = (((read_cntpct_el0() - el3_time_base)) / SYS_COUNTER_FREQ_IN_MHZ)
			- align_time_base;
	return cval;
}

u_register_t uptime_sync_with_rich_os(u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *handle,
	struct smccc_res *smccc_ret)
{
	if (!x3) {
		el3_uptime_init(el3_uptime() - ((x1 + (x2 << 32))/1000), 0);
		MT_LOG_KTIME_SET();
	} else {
		MT_LOG_KTIME_CLEAR();
	}

	return 0;
}
DECLARE_SMC_HANDLER(MTK_SIP_KERNEL_TIME_SYNC, uptime_sync_with_rich_os);
