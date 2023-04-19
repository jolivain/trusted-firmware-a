/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <platform_def.h>

#include "mt_spm_internal.h"
#include "mt_spm_reg.h"

void spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc)
{
	uint32_t con1;

	mmio_write_32(SPMC_DORMANT_ENABLE, 0);

	/* disable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* disable pcm timer after leaving FW */
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~PCM_TIMER_EN_LSB));

	/* reset PCM */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_SW_RESET_LSB);
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
	if ((mmio_read_32(PCM_FSM_STA) & 0x7fffff) != PCM_FSM_STA_DEF)
		ERROR("reset pcm(PCM_FSM_STA=0x%x)\n", mmio_read_32(PCM_FSM_STA));

	/* init PCM_CON0 (disable event vector) */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | EN_IM_SLEEP_DVS_LSB);

	/* init PCM_CON1 */
	con1 = mmio_read_32(PCM_CON1);
	con1 = con1 | SPM_REGWR_CFG_KEY | MIF_APBEN_LSB |
	       (pcmdesc->replace ? 0 : IM_NONRP_EN_LSB) |
	       SPM_SRAM_ISOINT_B_LSB | EVENT_LOCK_EN_LSB |
	       SCP_APB_INTERNAL_EN_LSB | MCUPM_APB_INTERNAL_EN_LSB;
	mmio_write_32(PCM_CON1, con1);
}

void spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc)
{
	uint32_t ptr, len, con0;

	/* tell IM where is PCM code (use slave mode if code existed) */
	ptr = (unsigned int)(unsigned long)pcmdesc->base;
	len = pcmdesc->size - 1;
	if (mmio_read_32(PCM_IM_PTR) != ptr || mmio_read_32(PCM_IM_LEN) != len ||
	    pcmdesc->sess > 2) {
		mmio_write_32(PCM_IM_PTR, ptr);
		mmio_write_32(PCM_IM_LEN, len);
	} else {
		mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | IM_SLAVE_LSB);
	}

	/* kick IM to fetch (only toggle IM_KICK) */
	con0 = mmio_read_32(PCM_CON0) & ~(IM_KICK_L_LSB | PCM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | IM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
}

void spm_init_pcm_register(void)
{
	uint32_t pcm_pwr_io_en;

	/* init r0 with POWER_ON_VAL0 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL0));
	pcm_pwr_io_en = mmio_read_32(PCM_PWR_IO_EN);
	pcm_pwr_io_en |= PCM_RF_SYNC_R0;
	mmio_write_32(PCM_PWR_IO_EN, pcm_pwr_io_en);
	pcm_pwr_io_en &= ~PCM_RF_SYNC_R0;
	mmio_write_32(PCM_PWR_IO_EN, pcm_pwr_io_en);

	/* init r7 with POWER_ON_VAL1 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL1));
	pcm_pwr_io_en = mmio_read_32(PCM_PWR_IO_EN);
	pcm_pwr_io_en |= PCM_RF_SYNC_R7;
	mmio_write_32(PCM_PWR_IO_EN, pcm_pwr_io_en);
	pcm_pwr_io_en &= ~PCM_RF_SYNC_R7;
	mmio_write_32(PCM_PWR_IO_EN, pcm_pwr_io_en);
}

void spm_init_event_vector(const struct pcm_desc *pcmdesc)
{
	/* init event vector register */
	mmio_write_32(PCM_EVENT_VECTOR0, pcmdesc->vec0);
	mmio_write_32(PCM_EVENT_VECTOR1, pcmdesc->vec1);
	mmio_write_32(PCM_EVENT_VECTOR2, pcmdesc->vec2);
	mmio_write_32(PCM_EVENT_VECTOR3, pcmdesc->vec3);
	mmio_write_32(PCM_EVENT_VECTOR4, pcmdesc->vec4);
	mmio_write_32(PCM_EVENT_VECTOR5, pcmdesc->vec5);
	mmio_write_32(PCM_EVENT_VECTOR6, pcmdesc->vec6);
	mmio_write_32(PCM_EVENT_VECTOR7, pcmdesc->vec7);
	mmio_write_32(PCM_EVENT_VECTOR8, pcmdesc->vec8);
	mmio_write_32(PCM_EVENT_VECTOR9, pcmdesc->vec9);
	mmio_write_32(PCM_EVENT_VECTOR10, pcmdesc->vec10);
	mmio_write_32(PCM_EVENT_VECTOR11, pcmdesc->vec11);
	mmio_write_32(PCM_EVENT_VECTOR12, pcmdesc->vec12);
	mmio_write_32(PCM_EVENT_VECTOR13, pcmdesc->vec13);
	mmio_write_32(PCM_EVENT_VECTOR14, pcmdesc->vec14);
	mmio_write_32(PCM_EVENT_VECTOR15, pcmdesc->vec15);
}

void spm_kick_pcm_to_run(void)
{
	uint32_t con0, pcm_pwr_io_en;

	/* init register to match PCM expectation */
	mmio_write_32(SPM_MAS_PAUSE_MASK_B, 0xffffffff);
	mmio_write_32(SPM_MAS_PAUSE2_MASK_B, 0xffffffff);
	mmio_write_32(PCM_REG_DATA_INI, 0);

	/* enable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7);

	/* enable r0 and r7 to control power */
	pcm_pwr_io_en = (mmio_read_32(PCM_PWR_IO_EN) & (~0xff)) |
			 PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7;
	mmio_write_32(PCM_PWR_IO_EN, pcm_pwr_io_en);

	/* check IM ready */
	while ((mmio_read_32(PCM_FSM_STA) & (0x7 << 7)) != (0x4 << 7))
		;

	/* kick PCM to run (only toggle PCM_KICK) */
	con0 = mmio_read_32(PCM_CON0) & ~(IM_KICK_L_LSB | PCM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
}
