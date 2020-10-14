/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <rtc.h>


static void rtc_config_interface(uint32_t addr, uint16_t data,
			    uint16_t mask, uint16_t shift)
{
	uint16_t pmic_reg;

	pmic_reg = rtc_read(addr);

	pmic_reg &= ~(mask << shift);
	pmic_reg |= (data << shift);

	rtc_write(addr, pmic_reg);
}

static void rtc_disable_2sec_reboot(void)
{
	uint16_t reboot;

	reboot = (rtc_read(RTC_AL_SEC) & ~RTC_BBPU_2SEC_EN) &
		 ~RTC_BBPU_AUTO_PDN_SEL;
	rtc_write(RTC_AL_SEC, reboot);
	rtc_write_trigger();
}

static void rtc_xosc_write(uint16_t val, bool reload)
{
	uint16_t bbpu;

	rtc_write(RTC_OSC32CON, RTC_OSC32CON_UNLOCK1);
	rtc_busy_wait();
	rtc_write(RTC_OSC32CON, RTC_OSC32CON_UNLOCK2);
	rtc_busy_wait();

	rtc_write(RTC_OSC32CON, val);
	rtc_busy_wait();

	if (reload) {
		bbpu = rtc_read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
		rtc_write(RTC_BBPU, bbpu);
		rtc_write_trigger();
	}
}

static void rtc_enable_k_eosc(void)
{
	uint16_t alm_dow, alm_sec;

	/* Turning on eosc cali mode clock */
	rtc_config_interface(PMIC_RG_SCK_TOP_CKPDN_CON0_CLR, 1,
			PMIC_RG_RTC_EOSC32_CK_PDN_MASK,
			PMIC_RG_RTC_EOSC32_CK_PDN_SHIFT);

	alm_sec = rtc_read(RTC_AL_SEC) & (~RTC_LPD_OPT_MASK);
	rtc_write(RTC_AL_SEC, alm_sec);
	rtc_write_trigger();
	rtc_write(RTC_CON, RTC_LPD_EN);
	rtc_write_trigger();
	rtc_write(RTC_CON, RTC_LPD_RST);
	rtc_write_trigger();
	rtc_write(RTC_CON, RTC_LPD_EN);
	rtc_write_trigger();

	rtc_write(RTC_POWERKEY1, RTC_POWERKEY1_KEY);
	rtc_write(RTC_POWERKEY2, RTC_POWERKEY2_KEY);
	rtc_write_trigger();

	/* set RTC EOSC calibration period = 8sec */
	alm_dow = (rtc_read(RTC_AL_DOW) & (~RTC_RG_EOSC_CALI_TD_MASK))
				| RTC_RG_EOSC_CALI_TD_8SEC;
	rtc_write(RTC_AL_DOW, alm_dow);
	rtc_write_trigger();

	rtc_write(RTC_BBPU,
		  rtc_read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD);
	rtc_write_trigger();

	/* Enable K EOSC mode :use solution1 of eosc cali to fix mt6359p 32K*/
	rtc_write(RTC_AL_YEA, (((rtc_read(RTC_AL_YEA) | RTC_K_EOSC_RSV_0)
				& (~RTC_K_EOSC_RSV_1)) | (RTC_K_EOSC_RSV_2)));
	rtc_write_trigger();

	INFO("[RTC] RTC_enable_k_eosc\n");
}

void rtc_power_off_sequence(void)
{
	uint16_t bbpu;

	rtc_disable_2sec_reboot();
	rtc_enable_k_eosc();

	bbpu = RTC_BBPU_KEY | RTC_BBPU_PWREN;

	if (rtc_writeif_unlock()) {
		rtc_write(RTC_BBPU, bbpu | RTC_BBPU_RESET_ALARM | RTC_BBPU_RESET_SPAR);
		rtc_write(RTC_AL_MASK, RTC_AL_MASK_DOW);
		rtc_write_trigger();
		mdelay(1);

		bbpu = rtc_read(RTC_BBPU);

		if (bbpu & RTC_BBPU_RESET_ALARM || bbpu & RTC_BBPU_RESET_SPAR) {
			INFO("[RTC] timeout\n");
		}

		bbpu = rtc_read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
		rtc_write(RTC_BBPU, bbpu);
		rtc_write_trigger();
	}
}
