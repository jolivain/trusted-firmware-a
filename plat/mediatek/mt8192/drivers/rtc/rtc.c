/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <rtc.h>

static void RTC_Config_Interface(uint32_t addr, uint16_t data,
			    uint16_t MASK, uint16_t SHIFT)
{
	uint16_t pmic_reg = 0;

	pmic_reg = RTC_Read(addr);

	pmic_reg &= ~(MASK << SHIFT);
	pmic_reg |= (data << SHIFT);

	RTC_Write(addr, pmic_reg);
}

static void rtc_disable_2sec_reboot(void)
{
	uint16_t reboot;

	reboot = (RTC_Read(RTC_AL_SEC) & ~RTC_BBPU_2SEC_EN) &
		 ~RTC_BBPU_AUTO_PDN_SEL;
	RTC_Write(RTC_AL_SEC, reboot);
	RTC_Write_Trigger();
}

static void rtc_xosc_write(uint16_t val, bool reload)
{
	uint16_t bbpu;

	RTC_Write(RTC_OSC32CON, RTC_OSC32CON_UNLOCK1);
	rtc_busy_wait();
	RTC_Write(RTC_OSC32CON, RTC_OSC32CON_UNLOCK2);
	rtc_busy_wait();

	RTC_Write(RTC_OSC32CON, val);
	rtc_busy_wait();

	if (reload) {
		bbpu = RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
		RTC_Write(RTC_BBPU, bbpu);
		RTC_Write_Trigger();
	}
}

static void rtc_enable_k_eosc(void)
{
	uint16_t osc32;
	uint16_t alm_dow;

	/* Truning on eosc cali mode clock */
	RTC_Config_Interface(PMIC_RG_SCK_TOP_CKPDN_CON0_SET, 1,
			PMIC_RG_RTC_EOSC32_CK_PDN_MASK,
			PMIC_RG_RTC_EOSC32_CK_PDN_SHIFT);
	RTC_Config_Interface(PMIC_RG_SCK_TOP_CKPDN_CON0, 0,
			PMIC_RG_RTC_EOSC32_CK_PDN_MASK,
			PMIC_RG_RTC_EOSC32_CK_PDN_SHIFT);

	/* set RTC EOSC calibration period = 8sec */
	alm_dow = (RTC_Read(RTC_AL_DOW) & (~RTC_RG_EOSC_CALI_TD_MASK)) | RTC_RG_EOSC_CALI_TD_8SEC;
	RTC_Write(RTC_AL_DOW, alm_dow);

	RTC_Write(RTC_BBPU,
		  RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD);
	RTC_Write_Trigger();

	/* Enable K EOSC mode :use solution2 of eosc cali to fix mt6359p 32K*/
	RTC_Write(RTC_AL_YEA, ((RTC_Read(RTC_AL_YEA) | RTC_K_EOSC_RSV_0)
				| RTC_K_EOSC_RSV_1 | RTC_K_EOSC_RSV_2));
	RTC_Write_Trigger();

	osc32 = RTC_Read(RTC_OSC32CON);
	rtc_xosc_write(osc32 | RTC_EMBCK_SRC_SEL, true);
	INFO("[RTC] RTC_enable_k_eosc\n");
}

void rtc_power_off_sequence(void)
{
	uint16_t bbpu;

	rtc_disable_2sec_reboot();
	rtc_enable_k_eosc();

	bbpu = RTC_BBPU_KEY | RTC_BBPU_PWREN;

	if (Writeif_unlock()) {
		RTC_Write(RTC_BBPU, bbpu | RTC_BBPU_RESET_ALARM | RTC_BBPU_RESET_SPAR);
		RTC_Write(RTC_AL_MASK, RTC_AL_MASK_DOW);
		RTC_Write_Trigger();
		mdelay(1);

		bbpu = RTC_Read(RTC_BBPU);

		if (bbpu & RTC_BBPU_RESET_ALARM || bbpu & RTC_BBPU_RESET_SPAR)
			INFO("[RTC] timeout\n");

		bbpu = RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
		RTC_Write(RTC_BBPU, bbpu);
		RTC_Write_Trigger();
		INFO("[RTC] BBPU=0x%x, IRQ_EN=0x%x, AL_MSK=0x%x, AL_SEC=0x%x\n",
		     RTC_Read(RTC_BBPU), RTC_Read(RTC_IRQ_EN),
		     RTC_Read(RTC_AL_MASK), RTC_Read(RTC_AL_SEC));
	}
}
