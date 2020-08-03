#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_pmic_wrap.h>
#include <platform_def.h>
#include <plat_pm.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**************************************
 * Define and Declare
 **************************************/
#define	SPM_INIT_DONE_US	20 /* Simulation result */

const char *wakeup_src_str[32] = {
	[0] = "R12_PCM_TIMER",
	[1] = "R12_RESERVED_DEBUG_B",
	[2] = "R12_KP_IRQ_B",
	[3] = "R12_APWDT_EVENT_B",
	[4] = "R12_APXGPT1_EVENT_B",
	[5] = "R12_CONN2AP_SPM_WAKEUP_B",
	[6] = "R12_EINT_EVENT_B",
	[7] = "R12_CONN_WDT_IRQ_B",
	[8] = "R12_CCIF0_EVENT_B",
	[9] = "R12_LOWBATTERY_IRQ_B",
	[10] = "R12_SC_SSPM2SPM_WAKEUP_B",
	[11] = "R12_SC_SCP2SPM_WAKEUP_B",
	[12] = "R12_SC_ADSP2SPM_WAKEUP_B",
	[13] = "R12_PCM_WDT_WAKEUP_B",
	[14] = "R12_USB_CDSC_B",
	[15] = "R12_USB_POWERDWN_B",
	[16] = "R12_SYS_TIMER_EVENT_B",
	[17] = "R12_EINT_EVENT_SECURE_B",
	[18] = "R12_CCIF1_EVENT_B",
	[19] = "R12_UART0_IRQ_B",
	[20] = "R12_AFE_IRQ_MCU_B",
	[21] = "R12_THERM_CTRL_EVENT_B",
	[22] = "R12_SYS_CIRQ_IRQ_B",
	[23] = "R12_MD2AP_PEER_EVENT_B",
	[24] = "R12_CSYSPWREQ_B",
	[25] = "R12_MD1_WDT_B",
	[26] = "R12_AP2AP_PEER_WAKEUPEVENT_B",
	[27] = "R12_SEJ_EVENT_B",
	[28] = "R12_SPM_CPU_WAKEUPEVENT_B",
	[29] = "R12_APUSYS",
	[30] = "R12_PCIE_BRIDGE_IRQ",
	[31] = "R12_PCIE_IRQ",
};

/**************************************
 * Function and API
 **************************************/

wake_reason_t __spm_output_wake_reason(int state_id,
				       const struct wake_status *wakesta)
{
	uint32_t i, bk_vtcxo_dur, spm_26m_off_pct = 0;
	wake_reason_t wr = WR_UNKNOWN;

	if (!IS_PLAT_SUSPEND_ID(state_id) || !wakesta)
		return WR_UNKNOWN;

	if (wakesta->tr.comm.r12 & R12_PCM_TIMER) {

		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER_EVENT)
			wr = WR_PCM_TIMER;
	}

	if (wakesta->tr.comm.r12 & R12_TWAM_IRQ_B) {

		if (wakesta->wake_misc & WAKE_MISC_DVFSRC_IRQ)
			wr = WR_DVFSRC;

		if (wakesta->wake_misc & WAKE_MISC_TWAM_IRQ_B)
			wr = WR_TWAM;

		if (wakesta->wake_misc & WAKE_MISC_PMSR_IRQ_B_SET0)
			wr = WR_PMSR;

		if (wakesta->wake_misc & WAKE_MISC_PMSR_IRQ_B_SET1)
			wr = WR_PMSR;

		if (wakesta->wake_misc & WAKE_MISC_PMSR_IRQ_B_SET2)
			wr = WR_PMSR;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_0)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_1)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_2)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_3)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_ALL)
			wr = WR_SPM_ACK_CHK;
	}

	if (wakesta->is_abort) {
		ERROR("spmfw flow is aborted, timer_out = %u\n",
		      wakesta->timer_out);
	} else {
		for (i = 0; i < 32; i++) {
			if (wakesta->r12 & (1U << i)) {
				INFO("wake up by %s, timer_out = %u\n",
				     wakeup_src_str[i], wakesta->timer_out);
				wr = WR_WAKE_SRC;
				break;
			}
		}
	}

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

	if (wakesta->timer_out != 0) {
		bk_vtcxo_dur = mmio_read_32(SPM_BK_VTCXO_DUR);
		spm_26m_off_pct = (100 * bk_vtcxo_dur) / wakesta->timer_out;
	}

	INFO("spm_26m_off_pct = %u\n", spm_26m_off_pct);

	return wr;
}

void __spm_set_cpu_status(int cpu)
{
	if (cpu >= 0 && cpu < 8) {
		mmio_write_32(ROOT_CPUTOP_ADDR, (1U << cpu));
		mmio_write_32(ROOT_CORE_ADDR, SPM_CPU0_PWR_CON + (cpu * 0x4) + 0x20000000);
		/* Notify MCUPM that preferred cpu wakeup */
		mmio_write_32(MCUPM_MBOX_WAKEUP_CPU, cpu);
	} else {
		ERROR("%s: error cpu number %d\n", __func__, cpu);
	}
}

static void spm_code_swapping(void)
{
	uint32_t con1;
	/* int retry = 0, timeout = 5000; */

	con1 = mmio_read_32(SPM_WAKEUP_EVENT_MASK);

	mmio_write_32(SPM_WAKEUP_EVENT_MASK, (con1 & ~(0x1)));
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, con1);
}

void __spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc)
{
	uint32_t con1;
	unsigned char first_load_fw = true;

	/* check the SPM FW is run or not */
	if (mmio_read_32(MD32PCM_CFGREG_SW_RSTN) & 0x1)
		first_load_fw = false;

	if (!first_load_fw) {
		/* SPM code swapping */
		spm_code_swapping();

		/* Backup PCM r0 -> SPM_POWER_ON_VAL0 before `reset PCM` */
		mmio_write_32(SPM_POWER_ON_VAL0, mmio_read_32(PCM_REG0_DATA));
	}

	/* disable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* disable pcm timer after leaving FW */
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~RG_PCM_TIMER_EN_LSB));

	/* reset PCM */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_SW_RESET_LSB);
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);

	/* init PCM_CON1 (disable PCM timer but keep PCM WDT setting) */
	con1 = mmio_read_32(PCM_CON1) & (RG_PCM_WDT_WAKE_LSB);
	mmio_write_32(PCM_CON1, con1 | SPM_REGWR_CFG_KEY | REG_EVENT_LOCK_EN_LSB |
			REG_SPM_SRAM_ISOINT_B_LSB | RG_AHBMIF_APBEN_LSB | REG_MD32_APB_INTERNAL_EN_LSB);
}

void __spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc)
{

	uint32_t con0;
	uint32_t pmem_words;
	uint32_t total_words;
	uint32_t pmem_start;
	uint32_t dmem_start;
	uint32_t ptr;

	/* tell IM where is PCM code (use slave mode if code existed) */
	ptr = pcmdesc->base_dma + 0x40000000;
	pmem_words = pcmdesc->pmem_words;
	total_words = pcmdesc->total_words;
	pmem_start = pcmdesc->pmem_start;
	dmem_start = pcmdesc->dmem_start;

	if (mmio_read_32(MD32PCM_DMA0_SRC) != ptr
			|| mmio_read_32(MD32PCM_DMA0_DST) != pmem_start
			|| mmio_read_32(MD32PCM_DMA0_WPPT) != pmem_words
			|| mmio_read_32(MD32PCM_DMA0_WPTO) != dmem_start
			|| mmio_read_32(MD32PCM_DMA0_COUNT) !=  total_words
			|| mmio_read_32(MD32PCM_DMA0_CON) !=  0x0003820E) {
		mmio_write_32(MD32PCM_DMA0_SRC, ptr);
		mmio_write_32(MD32PCM_DMA0_DST, pmem_start);
		mmio_write_32(MD32PCM_DMA0_WPPT, pmem_words);
		mmio_write_32(MD32PCM_DMA0_WPTO, dmem_start);
		mmio_write_32(MD32PCM_DMA0_COUNT, total_words);
		mmio_write_32(MD32PCM_DMA0_CON, 0x0003820E);
		mmio_write_32(MD32PCM_DMA0_START, 0x00008000);
	} else {
		mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | RG_IM_SLAVE_LSB);
	}

	/* kick IM to fetch (only toggle IM_KICK) */
	con0 = mmio_read_32(PCM_CON0);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
}

void __spm_init_pcm_register(void)
{
	/* init r0 with POWER_ON_VAL0 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL0));
	mmio_write_32(PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* init r7 with POWER_ON_VAL1 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL1));
	mmio_write_32(PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
	mmio_write_32(PCM_PWR_IO_EN, 0);
}

void __spm_src_req_update(const struct pwr_ctrl *pwrctrl, unsigned int resource_usage)
{

	uint8_t reg_spm_apsrc_req = (resource_usage & MT_SPM_DRAM_S0)
						? 1 : pwrctrl->reg_spm_apsrc_req;
	uint8_t reg_spm_ddr_en_req = (resource_usage & MT_SPM_DRAM_S1)
						? 1 : pwrctrl->reg_spm_ddr_en_req;
	uint8_t reg_spm_vrf18_req = (resource_usage & MT_SPM_SYSPLL)
						? 1 : pwrctrl->reg_spm_vrf18_req;
	uint8_t reg_spm_infra_req = (resource_usage & MT_SPM_INFRA)
						? 1 : pwrctrl->reg_spm_infra_req;
	uint8_t reg_spm_f26m_req  = (resource_usage & (MT_SPM_26M | MT_SPM_XO_FPM))
						? 1 : pwrctrl->reg_spm_f26m_req;

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((reg_spm_apsrc_req & 0x1) << 0) |
		((reg_spm_f26m_req & 0x1) << 1) |
		((reg_spm_infra_req & 0x1) << 3) |
		((reg_spm_vrf18_req & 0x1) << 4) |
		((reg_spm_ddr_en_req & 0x1) << 7) |
		((pwrctrl->reg_spm_dvfs_req & 0x1) << 8) |
		((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 9) |
		((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 10) |
		((pwrctrl->reg_spm_adsp_mailbox_req & 0x1) << 11) |
		((pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 12));
}

void __spm_set_power_control(const struct pwr_ctrl *pwrctrl)
{
	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
	mmio_write_32(SPM_AP_STANDBY_CON,
		((pwrctrl->reg_wfi_op & 0x1) << 0) |
		((pwrctrl->reg_wfi_type & 0x1) << 1) |
		((pwrctrl->reg_mp0_cputop_idle_mask & 0x1) << 2) |
		((pwrctrl->reg_mp1_cputop_idle_mask & 0x1) << 3) |
		((pwrctrl->reg_mcusys_idle_mask & 0x1) << 4) |
		((pwrctrl->reg_md_apsrc_1_sel & 0x1) << 25) |
		((pwrctrl->reg_md_apsrc_0_sel & 0x1) << 26) |
		((pwrctrl->reg_conn_apsrc_sel & 0x1) << 29));

	/* SPM_SRC6_MASK */
	mmio_write_32(SPM_SRC6_MASK,
		((pwrctrl->reg_dpmaif_srcclkena_mask_b & 0x1) << 0) |
		((pwrctrl->reg_dpmaif_infra_req_mask_b & 0x1) << 1) |
		((pwrctrl->reg_dpmaif_apsrc_req_mask_b & 0x1) << 2) |
		((pwrctrl->reg_dpmaif_vrf18_req_mask_b & 0x1) << 3) |
		((pwrctrl->reg_dpmaif_ddr_en_mask_b & 0x1) << 4));

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((pwrctrl->reg_spm_apsrc_req & 0x1) << 0) |
		((pwrctrl->reg_spm_f26m_req & 0x1) << 1) |
		((pwrctrl->reg_spm_infra_req & 0x1) << 3) |
		((pwrctrl->reg_spm_vrf18_req & 0x1) << 4) |
		((pwrctrl->reg_spm_ddr_en_req & 0x1) << 7) |
		((pwrctrl->reg_spm_dvfs_req & 0x1) << 8) |
		((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 9) |
		((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 10) |
		((pwrctrl->reg_spm_adsp_mailbox_req & 0x1) << 11) |
		((pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 12));

	/* SPM_SRC_MASK */
	mmio_write_32(SPM_SRC_MASK,
		((pwrctrl->reg_md_srcclkena_0_mask_b & 0x1) << 0) |
		((pwrctrl->reg_md_srcclkena2infra_req_0_mask_b & 0x1) << 1) |
		((pwrctrl->reg_md_apsrc2infra_req_0_mask_b & 0x1) << 2) |
		((pwrctrl->reg_md_apsrc_req_0_mask_b & 0x1) << 3) |
		((pwrctrl->reg_md_vrf18_req_0_mask_b & 0x1) << 4) |
		((pwrctrl->reg_md_ddr_en_0_mask_b & 0x1) << 5) |
		((pwrctrl->reg_md_srcclkena_1_mask_b & 0x1) << 6) |
		((pwrctrl->reg_md_srcclkena2infra_req_1_mask_b & 0x1) << 7) |
		((pwrctrl->reg_md_apsrc2infra_req_1_mask_b & 0x1) << 8) |
		((pwrctrl->reg_md_apsrc_req_1_mask_b & 0x1) << 9) |
		((pwrctrl->reg_md_vrf18_req_1_mask_b & 0x1) << 10) |
		((pwrctrl->reg_md_ddr_en_1_mask_b & 0x1) << 11) |
		((pwrctrl->reg_conn_srcclkena_mask_b & 0x1) << 12) |
		((pwrctrl->reg_conn_srcclkenb_mask_b & 0x1) << 13) |
		((pwrctrl->reg_conn_infra_req_mask_b & 0x1) << 14) |
		((pwrctrl->reg_conn_apsrc_req_mask_b & 0x1) << 15) |
		((pwrctrl->reg_conn_vrf18_req_mask_b & 0x1) << 16) |
		((pwrctrl->reg_conn_ddr_en_mask_b & 0x1) << 17) |
		((pwrctrl->reg_conn_vfe28_mask_b & 0x1) << 18) |
		((pwrctrl->reg_srcclkeni0_srcclkena_mask_b & 0x1) << 19) |
		((pwrctrl->reg_srcclkeni0_infra_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_srcclkeni1_srcclkena_mask_b & 0x1) << 21) |
		((pwrctrl->reg_srcclkeni1_infra_req_mask_b & 0x1) << 22) |
		((pwrctrl->reg_srcclkeni2_srcclkena_mask_b & 0x1) << 23) |
		((pwrctrl->reg_srcclkeni2_infra_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_infrasys_apsrc_req_mask_b & 0x1) << 25) |
		((pwrctrl->reg_infrasys_ddr_en_mask_b & 0x1) << 26) |
		((pwrctrl->reg_md32_srcclkena_mask_b & 0x1) << 27) |
		((pwrctrl->reg_md32_infra_req_mask_b & 0x1) << 28) |
		((pwrctrl->reg_md32_apsrc_req_mask_b & 0x1) << 29) |
		((pwrctrl->reg_md32_vrf18_req_mask_b & 0x1) << 30) |
		((pwrctrl->reg_md32_ddr_en_mask_b & 0x1) << 31));

	/* SPM_SRC2_MASK */
	mmio_write_32(SPM_SRC2_MASK,
		((pwrctrl->reg_scp_srcclkena_mask_b & 0x1) << 0) |
		((pwrctrl->reg_scp_infra_req_mask_b & 0x1) << 1) |
		((pwrctrl->reg_scp_apsrc_req_mask_b & 0x1) << 2) |
		((pwrctrl->reg_scp_vrf18_req_mask_b & 0x1) << 3) |
		((pwrctrl->reg_scp_ddr_en_mask_b & 0x1) << 4) |
		((pwrctrl->reg_audio_dsp_srcclkena_mask_b & 0x1) << 5) |
		((pwrctrl->reg_audio_dsp_infra_req_mask_b & 0x1) << 6) |
		((pwrctrl->reg_audio_dsp_apsrc_req_mask_b & 0x1) << 7) |
		((pwrctrl->reg_audio_dsp_vrf18_req_mask_b & 0x1) << 8) |
		((pwrctrl->reg_audio_dsp_ddr_en_mask_b & 0x1) << 9) |
		((pwrctrl->reg_ufs_srcclkena_mask_b & 0x1) << 10) |
		((pwrctrl->reg_ufs_infra_req_mask_b & 0x1) << 11) |
		((pwrctrl->reg_ufs_apsrc_req_mask_b & 0x1) << 12) |
		((pwrctrl->reg_ufs_vrf18_req_mask_b & 0x1) << 13) |
		((pwrctrl->reg_ufs_ddr_en_mask_b & 0x1) << 14) |
		((pwrctrl->reg_disp0_apsrc_req_mask_b & 0x1) << 15) |
		((pwrctrl->reg_disp0_ddr_en_mask_b & 0x1) << 16) |
		((pwrctrl->reg_disp1_apsrc_req_mask_b & 0x1) << 17) |
		((pwrctrl->reg_disp1_ddr_en_mask_b & 0x1) << 18) |
		((pwrctrl->reg_gce_infra_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_gce_apsrc_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_gce_vrf18_req_mask_b & 0x1) << 21) |
		((pwrctrl->reg_gce_ddr_en_mask_b & 0x1) << 22) |
		((pwrctrl->reg_apu_srcclkena_mask_b & 0x1) << 23) |
		((pwrctrl->reg_apu_infra_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_apu_apsrc_req_mask_b & 0x1) << 25) |
		((pwrctrl->reg_apu_vrf18_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_apu_ddr_en_mask_b & 0x1) << 27) |
		((pwrctrl->reg_cg_check_srcclkena_mask_b & 0x1) << 28) |
		((pwrctrl->reg_cg_check_apsrc_req_mask_b & 0x1) << 29) |
		((pwrctrl->reg_cg_check_vrf18_req_mask_b & 0x1) << 30) |
		((pwrctrl->reg_cg_check_ddr_en_mask_b & 0x1) << 31));

	/* SPM_SRC3_MASK */
	mmio_write_32(SPM_SRC3_MASK,
		((pwrctrl->reg_dvfsrc_event_trigger_mask_b & 0x1) << 0) |
		((pwrctrl->reg_sw2spm_int0_mask_b & 0x1) << 1) |
		((pwrctrl->reg_sw2spm_int1_mask_b & 0x1) << 2) |
		((pwrctrl->reg_sw2spm_int2_mask_b & 0x1) << 3) |
		((pwrctrl->reg_sw2spm_int3_mask_b & 0x1) << 4) |
		((pwrctrl->reg_sc_adsp2spm_wakeup_mask_b & 0x1) << 5) |
		((pwrctrl->reg_sc_sspm2spm_wakeup_mask_b & 0xf) << 6) |
		((pwrctrl->reg_sc_scp2spm_wakeup_mask_b & 0x1) << 10) |
		((pwrctrl->reg_csyspwrreq_mask & 0x1) << 11) |
		((pwrctrl->reg_spm_srcclkena_reserved_mask_b & 0x1) << 12) |
		((pwrctrl->reg_spm_infra_req_reserved_mask_b & 0x1) << 13) |
		((pwrctrl->reg_spm_apsrc_req_reserved_mask_b & 0x1) << 14) |
		((pwrctrl->reg_spm_vrf18_req_reserved_mask_b & 0x1) << 15) |
		((pwrctrl->reg_spm_ddr_en_reserved_mask_b & 0x1) << 16) |
		((pwrctrl->reg_mcupm_srcclkena_mask_b & 0x1) << 17) |
		((pwrctrl->reg_mcupm_infra_req_mask_b & 0x1) << 18) |
		((pwrctrl->reg_mcupm_apsrc_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_mcupm_vrf18_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_mcupm_ddr_en_mask_b & 0x1) << 21) |
		((pwrctrl->reg_msdc0_srcclkena_mask_b & 0x1) << 22) |
		((pwrctrl->reg_msdc0_infra_req_mask_b & 0x1) << 23) |
		((pwrctrl->reg_msdc0_apsrc_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_msdc0_vrf18_req_mask_b & 0x1) << 25) |
		((pwrctrl->reg_msdc0_ddr_en_mask_b & 0x1) << 26) |
		((pwrctrl->reg_msdc1_srcclkena_mask_b & 0x1) << 27) |
		((pwrctrl->reg_msdc1_infra_req_mask_b & 0x1) << 28) |
		((pwrctrl->reg_msdc1_apsrc_req_mask_b & 0x1) << 29) |
		((pwrctrl->reg_msdc1_vrf18_req_mask_b & 0x1) << 30) |
		((pwrctrl->reg_msdc1_ddr_en_mask_b & 0x1) << 31));

	/* SPM_SRC4_MASK */
	mmio_write_32(SPM_SRC4_MASK,
		((pwrctrl->ccif_event_mask_b & 0xffff) << 0) |
		((pwrctrl->reg_bak_psri_srcclkena_mask_b & 0x1) << 16) |
		((pwrctrl->reg_bak_psri_infra_req_mask_b & 0x1) << 17) |
		((pwrctrl->reg_bak_psri_apsrc_req_mask_b & 0x1) << 18) |
		((pwrctrl->reg_bak_psri_vrf18_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_bak_psri_ddr_en_mask_b & 0x1) << 20) |
		((pwrctrl->reg_dramc0_md32_infra_req_mask_b & 0x1) << 21) |
		((pwrctrl->reg_dramc0_md32_vrf18_req_mask_b & 0x1) << 22) |
		((pwrctrl->reg_dramc1_md32_infra_req_mask_b & 0x1) << 23) |
		((pwrctrl->reg_dramc1_md32_vrf18_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_conn_srcclkenb2pwrap_mask_b & 0x1) << 25) |
		((pwrctrl->reg_dramc0_md32_wakeup_mask & 0x1) << 26) |
		((pwrctrl->reg_dramc1_md32_wakeup_mask & 0x1) << 27));

	/* SPM_SRC5_MASK */
	mmio_write_32(SPM_SRC5_MASK,
		((pwrctrl->reg_mcusys_merge_apsrc_req_mask_b & 0x1ff) << 0) |
		((pwrctrl->reg_mcusys_merge_ddr_en_mask_b & 0x1ff) << 9) |
		((pwrctrl->reg_msdc2_srcclkena_mask_b & 0x1) << 18) |
		((pwrctrl->reg_msdc2_infra_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_msdc2_apsrc_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_msdc2_vrf18_req_mask_b & 0x1) << 21) |
		((pwrctrl->reg_msdc2_ddr_en_mask_b & 0x1) << 22) |
		((pwrctrl->reg_pcie_srcclkena_mask_b & 0x1) << 23) |
		((pwrctrl->reg_pcie_infra_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_pcie_apsrc_req_mask_b & 0x1) << 25) |
		((pwrctrl->reg_pcie_vrf18_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_pcie_ddr_en_mask_b & 0x1) << 27));

	/* SPM_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		((pwrctrl->reg_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_EXT_MASK,
		((pwrctrl->reg_ext_wakeup_event_mask & 0xffffffff) << 0));

	/* Auto-gen End */
}

void __spm_disable_pcm_timer(void)
{
	mmio_clrsetbits_32(PCM_CON1, RG_PCM_TIMER_EN_LSB, SPM_REGWR_CFG_KEY);
}

void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	uint32_t val, mask, isr;

	/* toggle event counter clear */
	mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | SPM_EVENT_COUNTER_CLR_LSB);
	/* toggle for reset SYS TIMER start point */
	mmio_write_32(SYS_TIMER_CON, mmio_read_32(SYS_TIMER_CON) | SYS_TIMER_START_EN_LSB);

	if (pwrctrl->timer_val_cust == 0)
		val = pwrctrl->timer_val;
	else
		val = pwrctrl->timer_val_cust;

	mmio_write_32(PCM_TIMER_VAL, val);
	mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | RG_PCM_TIMER_EN_LSB);

	/* unmask AP wakeup source */
	if (pwrctrl->wake_src_cust == 0)
		mask = pwrctrl->wake_src;
	else
		mask = pwrctrl->wake_src_cust;

	if (pwrctrl->reg_csyspwrreq_mask)
		mask &= ~R12_CSYSPWREQ_B;
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~mask);

	/* unmask SPM ISR (keep TWAM setting) */
	isr = mmio_read_32(SPM_IRQ_MASK);
	mmio_write_32(SPM_IRQ_MASK, isr | ISRM_RET_IRQ_AUX);

	/* toggle event counter clear */
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~SPM_EVENT_COUNTER_CLR_LSB));
	/* toggle for reset SYS TIMER start point */
	mmio_write_32(SYS_TIMER_CON, mmio_read_32(SYS_TIMER_CON) & ~SYS_TIMER_START_EN_LSB);
}

void __spm_set_fw_resume_option(struct pwr_ctrl *pwrctrl)
{
#if SPM_FW_NO_RESUME
    /* do Nothing */
#else
	pwrctrl->pcm_flags1 |= SPM_FLAG1_DISABLE_NO_RESUME;
#endif
}

void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl)
{
	/* set PCM flags and data */
	if (pwrctrl->pcm_flags_cust_clr != 0)
		pwrctrl->pcm_flags &= ~pwrctrl->pcm_flags_cust_clr;
	if (pwrctrl->pcm_flags_cust_set != 0)
		pwrctrl->pcm_flags |= pwrctrl->pcm_flags_cust_set;
	if (pwrctrl->pcm_flags1_cust_clr != 0)
		pwrctrl->pcm_flags1 &= ~pwrctrl->pcm_flags1_cust_clr;
	if (pwrctrl->pcm_flags1_cust_set != 0)
		pwrctrl->pcm_flags1 |= pwrctrl->pcm_flags1_cust_set;

	mmio_write_32(SPM_SW_FLAG_0, pwrctrl->pcm_flags);

	mmio_write_32(SPM_SW_FLAG_1, pwrctrl->pcm_flags1);

	mmio_write_32(SPM_SW_RSV_7, pwrctrl->pcm_flags);

	mmio_write_32(SPM_SW_RSV_8, pwrctrl->pcm_flags1);
}

void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl)
{
	uint32_t con0;

	/* Waiting for loading SPMFW done*/
	while (mmio_read_32(MD32PCM_DMA0_RLCT) != 0x0)
		;

	/* FIXME: init register to match PCM expectation */
	mmio_write_32(SPM_BUS_PROTECT_MASK_B, 0xffffffff);
	mmio_write_32(SPM_BUS_PROTECT2_MASK_B, 0xffffffff);
	mmio_write_32(PCM_REG_DATA_INI, 0);

	__spm_set_pcm_flags(pwrctrl);

	/* enable r0 and r7 to control power */
	/* mmio_write_32(PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7); */
	/* kick PCM to run (only toggle PCM_KICK) */
	con0 = mmio_read_32(PCM_CON0);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
	/* reset md32pcm */
	con0 = mmio_read_32(MD32PCM_CFGREG_SW_RSTN);
	mmio_write_32(MD32PCM_CFGREG_SW_RSTN, con0 | 0x1);

	/* Waiting for SPM init done and entering WFI*/
	udelay(SPM_INIT_DONE_US);
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

	if (ext_status & SPM_INTERNAL_STATUS_HW_S1) {
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
	wakesta->is_abort = wakesta->debug_flag & DEBUG_ABORT_MASK;
	wakesta->is_abort |= wakesta->debug_flag1 & DEBUG_ABORT_MASK_1;
}

void __spm_clean_after_wakeup(void)
{
	/*
	 * Copy SPM_WAKEUP_STA to SPM_BK_WAKE_EVENT
	 * before clear SPM_WAKEUP_STA
	 *
	 * CPU dormant driver @kernel will copy  edge-trig IRQ pending
	 * (recorded @SPM_BK_WAKE_EVENT) to GIC
	 */
	/* FIXME */
	mmio_write_32(SPM_BK_WAKE_EVENT, mmio_read_32(SPM_WAKEUP_STA) | mmio_read_32(SPM_BK_WAKE_EVENT));

	/* [Vcorefs] can not switch back to POWER_ON_VAL0 here,
	 *  the FW stays in VCORE DVFS which use r0 to Ctrl MEM
	 */
	/* disable r0 and r7 to control power */
	/* mmio_write_32(PCM_PWR_IO_EN, 0); */

	/* clean CPU wakeup event */
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);

	/* [Vcorefs] not disable pcm timer here, due to the
	 * following vcore dvfs will use it for latency check
	 */
	/* clean PCM timer event */
	/* mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~PCM_TIMER_EN_LSB)); */

	/* clean wakeup event raw status (for edge trigger event) */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, 0xefffffff); /* bit[28] for cpu wake up event */

	/* clean ISR status (except TWAM) */
	mmio_write_32(SPM_IRQ_MASK, mmio_read_32(SPM_IRQ_MASK) | ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL_EXC_TWAM);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);
}

void __spm_set_pcm_wdt(int en)
{
	/* enable PCM WDT (normal mode) to start count if needed */
	if (en) {
		uint32_t con1;

		con1 = mmio_read_32(PCM_CON1) & ~(RG_PCM_WDT_WAKE_LSB);
		mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | con1);

		if (mmio_read_32(PCM_TIMER_VAL) > PCM_TIMER_MAX)
			mmio_write_32(PCM_TIMER_VAL, PCM_TIMER_MAX);
		mmio_write_32(PCM_WDT_VAL, mmio_read_32(PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
		mmio_write_32(PCM_CON1, con1 | SPM_REGWR_CFG_KEY | RG_PCM_WDT_EN_LSB);
	} else {
		mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) &
		~RG_PCM_WDT_EN_LSB));
	}
}

void __spm_send_cpu_wakeup_event(void)
{
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
	/* SPM will clear SPM_CPU_WAKEUP_EVENT */
}

void __spm_ext_int_wakeup_req_clr(void)
{
	mmio_write_32(EXT_INT_WAKEUP_REQ_CLR, mmio_read_32(ROOT_CPUTOP_ADDR));

	/* Clear spm2mcupm wakeup interrupt status */
	mmio_write_32(SPM2MCUPM_CON, 0);
}

void __spm_clean_before_wfi(void)
{
}

static unsigned int mt_spm_bblpm_cnt;
void __spm_xo_soc_bblpm(int en)
{
	unsigned int val = mmio_read_32(RC_M00_SRCLKEN_CFG);

	val = en ? ((val | 0x8) & ~0x10)
		: ((val | 0x10) & ~0x8);

	if (en) {
		assert(mt_spm_bblpm_cnt == 0);
		mt_spm_bblpm_cnt += 1;
	} else
		mt_spm_bblpm_cnt -= 1;
	mmio_write_32(RC_M00_SRCLKEN_CFG, val);
}

void __spm_hw_s1_state_monitor(int en, unsigned int *status)
{
	unsigned int reg;

	if (en) {
		reg = mmio_read_32(SPM_ACK_CHK_CON_3);
		reg &= ~SPM_ACK_CHK_3_CON_CLR_ALL;
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
		reg |= SPM_ACK_CHK_3_CON_EN;
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
	} else {

		reg = mmio_read_32(SPM_ACK_CHK_CON_3);

		if (reg & SPM_ACK_CHK_3_CON_RESULT) {
			if (status)
				*status |= SPM_INTERNAL_STATUS_HW_S1;
		}

		reg |= (SPM_ACK_CHK_3_CON_HW_MODE_TRIG
			| SPM_ACK_CHK_3_CON_CLR_ALL);
		reg &= ~(SPM_ACK_CHK_3_CON_EN);
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
	}
}

