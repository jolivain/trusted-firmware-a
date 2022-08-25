/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <plat_pm.h>
#include <platform_def.h>

/* Define and Declare */
#define ROOT_CORE_ADDR_OFFSET			(0x20000000)
#define SPM_WAKEUP_EVENT_MASK_CLEAN_MASK	(0xefffffff)
#define SPM_INIT_DONE_US			(20)

static unsigned int mt_spm_bblpm_cnt;

void spm_dump_wakesta(const struct wake_status *wakesta)
{
	uint32_t bk_vtcxo_dur, spm_26m_off_pct;

	INFO("r12 = 0x%x, r12_ext = 0x%x, r13 = 0x%x, debug_flag = 0x%x 0x%x\n",
	     wakesta->r12, wakesta->r12_ext, wakesta->r13, wakesta->debug_flag,
	     wakesta->debug_flag1);
	INFO("raw_sta = 0x%x 0x%x 0x%x, idle_sta = 0x%x, cg_check_sta = 0x%x\n",
	     wakesta->raw_sta, wakesta->md32pcm_wakeup_sta,
	     wakesta->md32pcm_event_sta, wakesta->idle_sta,
	     wakesta->cg_check_sta);
	INFO("req_sta = 0x%x 0x%x 0x%x 0x%x 0x%x, isr = 0x%x\n",
	     wakesta->req_sta0, wakesta->req_sta1, wakesta->req_sta2,
	     wakesta->req_sta3, wakesta->req_sta4, wakesta->isr);
	INFO("rt_req_sta0 = 0x%x, rt_req_sta1 = 0x%x, rt_req_sta2 = 0x%x\n",
	     wakesta->rt_req_sta0, wakesta->rt_req_sta1, wakesta->rt_req_sta2);
	INFO("rt_req_sta3 = 0x%x, dram_sw_con_3 = 0x%x, raw_ext_sta = 0x%x\n",
	     wakesta->rt_req_sta3, wakesta->rt_req_sta4, wakesta->raw_ext_sta);
	INFO("wake_misc = 0x%x, pcm_flag = 0x%x 0x%x 0x%x 0x%x, req = 0x%x\n",
	     wakesta->wake_misc, wakesta->sw_flag0, wakesta->sw_flag1,
	     wakesta->b_sw_flag0, wakesta->b_sw_flag1, wakesta->src_req);
	INFO("clk_settle = 0x%x, wlk_cntcv_l = 0x%x, wlk_cntcv_h = 0x%x\n",
	     wakesta->clk_settle, mmio_read_32(SYS_TIMER_VALUE_L),
	     mmio_read_32(SYS_TIMER_VALUE_H));

	if (wakesta->timer_out != 0U) {
		bk_vtcxo_dur = mmio_read_32(SPM_BK_VTCXO_DUR);
		spm_26m_off_pct = (100 * bk_vtcxo_dur) / wakesta->timer_out;
		INFO("spm_26m_off_pct = %u\n", spm_26m_off_pct);
	}
}

/* Function and API */
void __spm_set_cpu_status(unsigned int cpu)
{
	uint32_t root_core_addr;

	if (cpu < 8U) {
		mmio_write_32(ROOT_CPUTOP_ADDR, BIT(cpu));

		root_core_addr = SPM_CPU0_PWR_CON + (cpu * 0x4);
		root_core_addr += ROOT_CORE_ADDR_OFFSET;
		mmio_write_32(ROOT_CORE_ADDR, root_core_addr);

		/* Notify SSPM that preferred cpu wakeup */
		mmio_write_32(MCUPM_MBOX_WAKEUP_CPU, cpu);
	} else {
		ERROR("%s: error cpu number %d\n", __func__, cpu);
	}
}

void __spm_disable_pcm_timer(void)
{
	mmio_clrsetbits_32(PCM_CON1, RG_PCM_TIMER_EN_LSB, SPM_REGWR_CFG_KEY);
}

void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl)
{
	/* set PCM flags and data */
	if (pwrctrl->pcm_flags_cust_clr != 0U) {
		pwrctrl->pcm_flags &= ~pwrctrl->pcm_flags_cust_clr;
	}

	if (pwrctrl->pcm_flags_cust_set != 0U) {
		pwrctrl->pcm_flags |= pwrctrl->pcm_flags_cust_set;
	}

	if (pwrctrl->pcm_flags1_cust_clr != 0U) {
		pwrctrl->pcm_flags1 &= ~pwrctrl->pcm_flags1_cust_clr;
	}

	if (pwrctrl->pcm_flags1_cust_set != 0U) {
		pwrctrl->pcm_flags1 |= pwrctrl->pcm_flags1_cust_set;
	}

	mmio_write_32(SPM_SW_FLAG_0, pwrctrl->pcm_flags);

	mmio_write_32(SPM_SW_FLAG_1, pwrctrl->pcm_flags1);

	mmio_write_32(SPM_SW_RSV_7, pwrctrl->pcm_flags);

	mmio_write_32(SPM_SW_RSV_8, pwrctrl->pcm_flags1);
}

void __spm_get_wakeup_status(struct wake_status *wakesta,
			     unsigned int ext_status)
{
	wakesta->tr.comm.r12 = mmio_read_32(SPM_BK_WAKE_EVENT);
	wakesta->tr.comm.timer_out = mmio_read_32(SPM_BK_PCM_TIMER);
	wakesta->tr.comm.r13 = mmio_read_32(PCM_REG13_DATA);
	wakesta->tr.comm.req_sta0 = mmio_read_32(SRC_REQ_STA_0);
	wakesta->tr.comm.req_sta1 = mmio_read_32(SRC_REQ_STA_1);
	wakesta->tr.comm.req_sta2 = mmio_read_32(SRC_REQ_STA_2);
	wakesta->tr.comm.req_sta3 = mmio_read_32(SRC_REQ_STA_3);
	wakesta->tr.comm.req_sta4 = mmio_read_32(SRC_REQ_STA_4);

	wakesta->tr.comm.debug_flag = mmio_read_32(PCM_WDT_LATCH_SPARE_0);
	wakesta->tr.comm.debug_flag1 = mmio_read_32(PCM_WDT_LATCH_SPARE_1);

	if ((ext_status & SPM_INTERNAL_STATUS_HW_S1) != 0U) {
		wakesta->tr.comm.debug_flag |= (SPM_DBG_DEBUG_IDX_DDREN_WAKE |
						SPM_DBG_DEBUG_IDX_DDREN_SLEEP);
		mmio_write_32(PCM_WDT_LATCH_SPARE_0,
			      wakesta->tr.comm.debug_flag);
	}

	wakesta->tr.comm.b_sw_flag0 = mmio_read_32(SPM_SW_RSV_7);
	wakesta->tr.comm.b_sw_flag1 = mmio_read_32(SPM_SW_RSV_8);

	/* record below spm info for debug */
	wakesta->r12 = mmio_read_32(SPM_BK_WAKE_EVENT);
	wakesta->r12_ext = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_sta = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_ext_sta = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->md32pcm_wakeup_sta = mmio_read_32(MD32PCM_WAKEUP_STA);
	wakesta->md32pcm_event_sta = mmio_read_32(MD32PCM_EVENT_STA);
	wakesta->src_req = mmio_read_32(SPM_SRC_REQ);

	/* backup of SPM_WAKEUP_MISC */
	wakesta->wake_misc = mmio_read_32(SPM_BK_WAKE_MISC);

	/* get sleep time, backup of PCM_TIMER_OUT */
	wakesta->timer_out = mmio_read_32(SPM_BK_PCM_TIMER);

	/* get other SYS and co-clock status */
	wakesta->r13 = mmio_read_32(PCM_REG13_DATA);
	wakesta->idle_sta = mmio_read_32(SUBSYS_IDLE_STA);
	wakesta->req_sta0 = mmio_read_32(SRC_REQ_STA_0);
	wakesta->req_sta1 = mmio_read_32(SRC_REQ_STA_1);
	wakesta->req_sta2 = mmio_read_32(SRC_REQ_STA_2);
	wakesta->req_sta3 = mmio_read_32(SRC_REQ_STA_3);
	wakesta->req_sta4 = mmio_read_32(SRC_REQ_STA_4);

	/* get HW CG check status */
	wakesta->cg_check_sta = mmio_read_32(SPM_CG_CHECK_STA);

	/* get debug flag for PCM execution check */
	wakesta->debug_flag = mmio_read_32(PCM_WDT_LATCH_SPARE_0);
	wakesta->debug_flag1 = mmio_read_32(PCM_WDT_LATCH_SPARE_1);

	/* get backup SW flag status */
	wakesta->b_sw_flag0 = mmio_read_32(SPM_SW_RSV_7);
	wakesta->b_sw_flag1 = mmio_read_32(SPM_SW_RSV_8);

	wakesta->rt_req_sta0 = mmio_read_32(SPM_SW_RSV_2);
	wakesta->rt_req_sta1 = mmio_read_32(SPM_SW_RSV_3);
	wakesta->rt_req_sta2 = mmio_read_32(SPM_SW_RSV_4);
	wakesta->rt_req_sta3 = mmio_read_32(SPM_SW_RSV_5);
	wakesta->rt_req_sta4 = mmio_read_32(SPM_SW_RSV_6);

	/* get ISR status */
	wakesta->isr = mmio_read_32(SPM_IRQ_STA);

	/* get SW flag status */
	wakesta->sw_flag0 = mmio_read_32(SPM_SW_FLAG_0);
	wakesta->sw_flag1 = mmio_read_32(SPM_SW_FLAG_1);

	/* get CLK SETTLE */
	wakesta->clk_settle = mmio_read_32(SPM_CLK_SETTLE);

	/* check abort */
	wakesta->abort = ((wakesta->debug_flag & DEBUG_ABORT_MASK) |
			  (wakesta->debug_flag1 & DEBUG_ABORT_MASK_1));
}

void __spm_clean_after_wakeup(void)
{
	mmio_write_32(SPM_BK_WAKE_EVENT,
		      (mmio_read_32(SPM_WAKEUP_STA) |
		       mmio_read_32(SPM_BK_WAKE_EVENT)));
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0U);

	/*
	 * clean wakeup event raw status (for edge trigger event)
	 * bit[28] for cpu wake up event
	 */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, SPM_WAKEUP_EVENT_MASK_CLEAN_MASK);

	/* clean ISR status (except TWAM) */
	mmio_setbits_32(SPM_IRQ_MASK, ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL_EXC_TWAM);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);
}

void __spm_set_pcm_wdt(int en)
{
	mmio_clrsetbits_32(PCM_CON1, RG_PCM_WDT_EN_LSB,
			   SPM_REGWR_CFG_KEY);

	if (en == 1) {
		mmio_clrsetbits_32(PCM_CON1, RG_PCM_WDT_WAKE_LSB,
				   SPM_REGWR_CFG_KEY);

		if (mmio_read_32(PCM_TIMER_VAL) > PCM_TIMER_MAX) {
			mmio_write_32(PCM_TIMER_VAL, PCM_TIMER_MAX);
		}

		mmio_write_32(PCM_WDT_VAL,
			      mmio_read_32(PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
		mmio_setbits_32(PCM_CON1,
				SPM_REGWR_CFG_KEY | RG_PCM_WDT_EN_LSB);
	}
}

void __spm_send_cpu_wakeup_event(void)
{
	/* SPM will clear SPM_CPU_WAKEUP_EVENT */
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
}

void __spm_xo_soc_bblpm(int en)
{
	if (en == 1) {
		mmio_clrsetbits_32(RC_M00_SRCLKEN_CFG,
				   RC_SW_SRCLKEN_FPM, RC_SW_SRCLKEN_RC);
		assert(mt_spm_bblpm_cnt == 0);
		mt_spm_bblpm_cnt += 1;
	} else {
		mmio_clrsetbits_32(RC_M00_SRCLKEN_CFG,
				   RC_SW_SRCLKEN_RC, RC_SW_SRCLKEN_FPM);
		mt_spm_bblpm_cnt -= 1;
	}
}

void __spm_hw_s1_state_monitor(int en, unsigned int *status)
{
	unsigned int reg;

	reg = mmio_read_32(SPM_ACK_CHK_CON_3);

	if (en == 1) {
		reg &= ~SPM_ACK_CHK_3_CON_CLR_ALL;
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
		reg |= SPM_ACK_CHK_3_CON_EN;
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
	} else {
		if (((reg & SPM_ACK_CHK_3_CON_RESULT) != 0U) &&
		    (status != NULL)) {
			*status |= SPM_INTERNAL_STATUS_HW_S1;
		}

		mmio_clrsetbits_32(SPM_ACK_CHK_CON_3, SPM_ACK_CHK_3_CON_EN,
				   SPM_ACK_CHK_3_CON_HW_MODE_TRIG |
				   SPM_ACK_CHK_3_CON_CLR_ALL);
	}
}
