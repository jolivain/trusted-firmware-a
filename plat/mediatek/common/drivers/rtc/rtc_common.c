/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>

#include <pmic_wrap_init.h>
#include <rtc.h>

/* RTC busy status polling interval and retry count */
enum {
	RTC_WRTGR_POLLING_DELAY_MS	= 10,
	RTC_WRTGR_POLLING_CNT		= 100
};

uint16_t rtc_read(uint32_t addr)
{
	uint32_t rdata = 0;

	pwrap_read((uint32_t)addr, &rdata);
	return (uint16_t)rdata;
}

void rtc_write(uint32_t addr, uint16_t data)
{
	pwrap_write((uint32_t)addr, (uint32_t)data);
}

int32_t rtc_busy_wait(void)
{
	uint64_t retry = RTC_WRTGR_POLLING_CNT;

	do {
		mdelay(RTC_WRTGR_POLLING_DELAY_MS);
		if (!(rtc_read(RTC_BBPU) & RTC_BBPU_CBUSY))
			return 1;
		retry--;
	} while (retry);

	ERROR("[RTC] rtc cbusy time out!\n");
	return 0;
}

int32_t rtc_write_trigger(void)
{
	rtc_write(RTC_WRTGR, 1);
	return rtc_busy_wait();
}

int32_t rtc_writeif_unlock(void)
{
	rtc_write(RTC_PROT, RTC_PROT_UNLOCK1);
	if (!rtc_write_trigger())
		return 0;
	rtc_write(RTC_PROT, RTC_PROT_UNLOCK2);
	if (!rtc_write_trigger())
		return 0;

	return 1;
}

