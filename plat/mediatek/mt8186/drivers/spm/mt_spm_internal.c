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

/* Function and API */
wake_reason_t __spm_output_wake_reason(int state_id,
				       const struct wake_status *wakesta)
{
	uint32_t i;
	wake_reason_t wr = WR_UNKNOWN;

	if (wakesta != NULL) {
		if (wakesta->abort != 0U) {
			ERROR("spmfw flow is aborted: 0x%x, timer_out = %u\n",
			      wakesta->abort, wakesta->timer_out);
		} else {
			for (i = 0U; i < 32U; i++) {
				if ((wakesta->r12 & BIT(i)) != 0U) {
					INFO("wake up by %s, timer_out = %u\n",
					     wakeup_src_str[i], wakesta->timer_out);
					wr = WR_WAKE_SRC;
					break;
				}
			}
		}
		spm_dump_wakesta(wakesta);
	}

	return wr;
}

void __spm_src_req_update(const struct pwr_ctrl *pwrctrl,
			  unsigned int resource_usage)
{
	uint8_t apsrc_req = ((resource_usage & MT_SPM_DRAM_S0) != 0U) ?
			     1 : pwrctrl->reg_spm_apsrc_req;
	uint8_t ddr_en_req = ((resource_usage & MT_SPM_DRAM_S1) != 0U) ?
			     1 : pwrctrl->reg_spm_ddren_req;
	uint8_t vrf18_req = ((resource_usage & MT_SPM_SYSPLL) != 0U) ?
			     1 : pwrctrl->reg_spm_vrf18_req;
	uint8_t infra_req = ((resource_usage & MT_SPM_INFRA) != 0U) ?
			     1 : pwrctrl->reg_spm_infra_req;
	uint8_t f26m_req  = ((resource_usage &
			      (MT_SPM_26M | MT_SPM_XO_FPM)) != 0U) ?
			    1 : pwrctrl->reg_spm_f26m_req;

	/*
	 * if SPM_FLAG_SSPM_INFRA_SLEEP_MODE set,
	 * clear sspm_srclkena_mask_b and sspm_infra_mask_b
	 */
	uint8_t reg_sspm_srcclkena_mask_b =
		(pwrctrl->pcm_flags & SPM_FLAG_SSPM_INFRA_SLEEP_MODE)
		 ? 0U : pwrctrl->reg_sspm_srcclkena_mask_b;

	uint8_t reg_sspm_infra_req_mask_b =
		(pwrctrl->pcm_flags & SPM_FLAG_SSPM_INFRA_SLEEP_MODE)
		 ? 0 : pwrctrl->reg_sspm_infra_req_mask_b;

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		      ((apsrc_req & 0x1) << 0) |
		      ((f26m_req & 0x1) << 1) |
		      ((infra_req & 0x1) << 3) |
		      ((vrf18_req & 0x1) << 4) |
		      ((ddr_en_req & 0x1) << 7) |
		      ((pwrctrl->reg_spm_dvfs_req & 0x1) << 8) |
		      ((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 9) |
		      ((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 10) |
		      ((pwrctrl->reg_spm_adsp_mailbox_req & 0x1) << 11) |
		      ((pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 12));

	/* SPM_SRC_MASK */
	mmio_write_32(SPM_SRC_MASK,
		      ((pwrctrl->reg_md_0_srcclkena_mask_b & 0x1) << 0) |
		      ((pwrctrl->reg_md_0_infra_req_mask_b & 0x1) << 1) |
		      ((pwrctrl->reg_md_0_apsrc_req_mask_b & 0x1) << 2) |
		      ((pwrctrl->reg_md_0_vrf18_req_mask_b & 0x1) << 3) |
		      ((pwrctrl->reg_md_0_ddren_req_mask_b & 0x1) << 4) |
		      ((pwrctrl->reg_md_1_srcclkena_mask_b & 0x1) << 5) |
		      ((pwrctrl->reg_md_1_infra_req_mask_b & 0x1) << 6) |
		      ((pwrctrl->reg_md_1_apsrc_req_mask_b & 0x1) << 7) |
		      ((pwrctrl->reg_md_1_vrf18_req_mask_b & 0x1) << 8) |
		      ((pwrctrl->reg_md_1_ddren_req_mask_b & 0x1) << 9) |
		      ((pwrctrl->reg_conn_srcclkena_mask_b & 0x1) << 10) |
		      ((pwrctrl->reg_conn_srcclkenb_mask_b & 0x1) << 11) |
		      ((pwrctrl->reg_conn_infra_req_mask_b & 0x1) << 12) |
		      ((pwrctrl->reg_conn_apsrc_req_mask_b & 0x1) << 13) |
		      ((pwrctrl->reg_conn_vrf18_req_mask_b & 0x1) << 14) |
		      ((pwrctrl->reg_conn_ddren_req_mask_b & 0x1) << 15) |
		      ((pwrctrl->reg_conn_vfe28_mask_b & 0x1) << 16) |
		      ((pwrctrl->reg_srcclkeni_srcclkena_mask_b & 0x7) << 17) |
		      ((pwrctrl->reg_srcclkeni_infra_req_mask_b & 0x7) << 20) |
		      ((pwrctrl->reg_infrasys_apsrc_req_mask_b & 0x1) << 25) |
		      ((pwrctrl->reg_infrasys_ddren_req_mask_b & 0x1) << 26) |
		      ((reg_sspm_srcclkena_mask_b & 0x1) << 27) |
		      ((reg_sspm_infra_req_mask_b & 0x1) << 28) |
		      ((pwrctrl->reg_sspm_apsrc_req_mask_b & 0x1) << 29) |
		      ((pwrctrl->reg_sspm_vrf18_req_mask_b & 0x1) << 30) |
		      ((pwrctrl->reg_sspm_ddren_req_mask_b & 0x1) << 31));
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
		      ((pwrctrl->reg_ccif_event_infra_req_mask_b & 0xffff) << 0) |
		      ((pwrctrl->reg_ccif_event_apsrc_req_mask_b & 0xffff) << 16));

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		      ((pwrctrl->reg_spm_apsrc_req & 0x1) << 0) |
		      ((pwrctrl->reg_spm_f26m_req & 0x1) << 1) |
		      ((pwrctrl->reg_spm_infra_req & 0x1) << 3) |
		      ((pwrctrl->reg_spm_vrf18_req & 0x1) << 4) |
		      ((pwrctrl->reg_spm_ddren_req & 0x1) << 7) |
		      ((pwrctrl->reg_spm_dvfs_req & 0x1) << 8) |
		      ((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 9) |
		      ((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 10) |
		      ((pwrctrl->reg_spm_adsp_mailbox_req & 0x1) << 11) |
		      ((pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 12));

	/* SPM_SRC_MASK */
	mmio_write_32(SPM_SRC_MASK,
		      ((pwrctrl->reg_md_0_srcclkena_mask_b & 0x1) << 0) |
		      ((pwrctrl->reg_md_0_infra_req_mask_b & 0x1) << 1) |
		      ((pwrctrl->reg_md_0_apsrc_req_mask_b & 0x1) << 2) |
		      ((pwrctrl->reg_md_0_vrf18_req_mask_b & 0x1) << 3) |
		      ((pwrctrl->reg_md_0_ddren_req_mask_b & 0x1) << 4) |
		      ((pwrctrl->reg_md_1_srcclkena_mask_b & 0x1) << 5) |
		      ((pwrctrl->reg_md_1_infra_req_mask_b & 0x1) << 6) |
		      ((pwrctrl->reg_md_1_apsrc_req_mask_b & 0x1) << 7) |
		      ((pwrctrl->reg_md_1_vrf18_req_mask_b & 0x1) << 8) |
		      ((pwrctrl->reg_md_1_ddren_req_mask_b & 0x1) << 9) |
		      ((pwrctrl->reg_conn_srcclkena_mask_b & 0x1) << 10) |
		      ((pwrctrl->reg_conn_srcclkenb_mask_b & 0x1) << 11) |
		      ((pwrctrl->reg_conn_infra_req_mask_b & 0x1) << 12) |
		      ((pwrctrl->reg_conn_apsrc_req_mask_b & 0x1) << 13) |
		      ((pwrctrl->reg_conn_vrf18_req_mask_b & 0x1) << 14) |
		      ((pwrctrl->reg_conn_ddren_req_mask_b & 0x1) << 15) |
		      ((pwrctrl->reg_conn_vfe28_mask_b & 0x1) << 16) |
		      ((pwrctrl->reg_srcclkeni_srcclkena_mask_b & 0x7) << 17) |
		      ((pwrctrl->reg_srcclkeni_infra_req_mask_b & 0x7) << 20) |
		      ((pwrctrl->reg_infrasys_apsrc_req_mask_b & 0x1) << 25) |
		      ((pwrctrl->reg_infrasys_ddren_req_mask_b & 0x1) << 26) |
		      ((pwrctrl->reg_sspm_srcclkena_mask_b & 0x1) << 27) |
		      ((pwrctrl->reg_sspm_infra_req_mask_b & 0x1) << 28) |
		      ((pwrctrl->reg_sspm_apsrc_req_mask_b & 0x1) << 29) |
		      ((pwrctrl->reg_sspm_vrf18_req_mask_b & 0x1) << 30) |
		      ((pwrctrl->reg_sspm_ddren_req_mask_b & 0x1) << 31));

	/* SPM_SRC2_MASK */
	mmio_write_32(SPM_SRC2_MASK,
		((pwrctrl->reg_scp_srcclkena_mask_b & 0x1) << 0) |
		((pwrctrl->reg_scp_infra_req_mask_b & 0x1) << 1) |
		((pwrctrl->reg_scp_apsrc_req_mask_b & 0x1) << 2) |
		((pwrctrl->reg_scp_vrf18_req_mask_b & 0x1) << 3) |
		((pwrctrl->reg_scp_ddren_req_mask_b & 0x1) << 4) |
		((pwrctrl->reg_audio_dsp_srcclkena_mask_b & 0x1) << 5) |
		((pwrctrl->reg_audio_dsp_infra_req_mask_b & 0x1) << 6) |
		((pwrctrl->reg_audio_dsp_apsrc_req_mask_b & 0x1) << 7) |
		((pwrctrl->reg_audio_dsp_vrf18_req_mask_b & 0x1) << 8) |
		((pwrctrl->reg_audio_dsp_ddren_req_mask_b & 0x1) << 9) |
		((pwrctrl->reg_ufs_srcclkena_mask_b & 0x1) << 10) |
		((pwrctrl->reg_ufs_infra_req_mask_b & 0x1) << 11) |
		((pwrctrl->reg_ufs_apsrc_req_mask_b & 0x1) << 12) |
		((pwrctrl->reg_ufs_vrf18_req_mask_b & 0x1) << 13) |
		((pwrctrl->reg_ufs_ddren_req_mask_b & 0x1) << 14) |
		((pwrctrl->reg_disp0_apsrc_req_mask_b & 0x1) << 15) |
		((pwrctrl->reg_disp0_ddren_req_mask_b & 0x1) << 16) |
		((pwrctrl->reg_disp1_apsrc_req_mask_b & 0x1) << 17) |
		((pwrctrl->reg_disp1_ddren_req_mask_b & 0x1) << 18) |
		((pwrctrl->reg_gce_infra_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_gce_apsrc_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_gce_vrf18_req_mask_b & 0x1) << 21) |
		((pwrctrl->reg_gce_ddren_req_mask_b & 0x1) << 22) |
		((pwrctrl->reg_apu_srcclkena_mask_b & 0x1) << 23) |
		((pwrctrl->reg_apu_infra_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_apu_apsrc_req_mask_b & 0x1) << 25) |
		((pwrctrl->reg_apu_vrf18_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_apu_ddren_req_mask_b & 0x1) << 27) |
		((pwrctrl->reg_cg_check_srcclkena_mask_b & 0x1) << 28) |
		((pwrctrl->reg_cg_check_apsrc_req_mask_b & 0x1) << 29) |
		((pwrctrl->reg_cg_check_vrf18_req_mask_b & 0x1) << 30) |
		((pwrctrl->reg_cg_check_ddren_req_mask_b & 0x1) << 31));

	/* SPM_SRC3_MASK */
	mmio_write_32(SPM_SRC3_MASK,
		((pwrctrl->reg_dvfsrc_event_trigger_mask_b & 0x1) << 0) |
		((pwrctrl->reg_sw2spm_wakeup_mask_b & 0xf) << 1) |
		((pwrctrl->reg_adsp2spm_wakeup_mask_b & 0x1) << 5) |
		((pwrctrl->reg_sspm2spm_wakeup_mask_b & 0xf) << 6) |
		((pwrctrl->reg_scp2spm_wakeup_mask_b & 0x1) << 10) |
		((pwrctrl->reg_csyspwrup_ack_mask & 0x1) << 11) |
		((pwrctrl->reg_spm_reserved_srcclkena_mask_b & 0x1) << 12) |
		((pwrctrl->reg_spm_reserved_infra_req_mask_b & 0x1) << 13) |
		((pwrctrl->reg_spm_reserved_apsrc_req_mask_b & 0x1) << 14) |
		((pwrctrl->reg_spm_reserved_vrf18_req_mask_b & 0x1) << 15) |
		((pwrctrl->reg_spm_reserved_ddren_req_mask_b & 0x1) << 16) |
		((pwrctrl->reg_mcupm_srcclkena_mask_b & 0x1) << 17) |
		((pwrctrl->reg_mcupm_infra_req_mask_b & 0x1) << 18) |
		((pwrctrl->reg_mcupm_apsrc_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_mcupm_vrf18_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_mcupm_ddren_req_mask_b & 0x1) << 21) |
		((pwrctrl->reg_msdc0_srcclkena_mask_b & 0x1) << 22) |
		((pwrctrl->reg_msdc0_infra_req_mask_b & 0x1) << 23) |
		((pwrctrl->reg_msdc0_apsrc_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_msdc0_vrf18_req_mask_b & 0x1) << 25) |
		((pwrctrl->reg_msdc0_ddren_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_msdc1_srcclkena_mask_b & 0x1) << 27) |
		((pwrctrl->reg_msdc1_infra_req_mask_b & 0x1) << 28) |
		((pwrctrl->reg_msdc1_apsrc_req_mask_b & 0x1) << 29) |
		((pwrctrl->reg_msdc1_vrf18_req_mask_b & 0x1) << 30) |
		((pwrctrl->reg_msdc1_ddren_req_mask_b & 0x1) << 31));

	/* SPM_SRC4_MASK */
	mmio_write_32(SPM_SRC4_MASK,
		((pwrctrl->reg_ccif_event_srcclkena_mask_b & 0xffff) << 0) |
		((pwrctrl->reg_bak_psri_srcclkena_mask_b & 0x1) << 16) |
		((pwrctrl->reg_bak_psri_infra_req_mask_b & 0x1) << 17) |
		((pwrctrl->reg_bak_psri_apsrc_req_mask_b & 0x1) << 18) |
		((pwrctrl->reg_bak_psri_vrf18_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_bak_psri_ddren_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_dramc_md32_infra_req_mask_b & 0x3) << 21) |
		((pwrctrl->reg_dramc_md32_vrf18_req_mask_b & 0x3) << 23) |
		((pwrctrl->reg_conn_srcclkenb2pwrap_mask_b & 0x1) << 25) |
		((pwrctrl->reg_dramc_md32_apsrc_req_mask_b & 0x3) << 26));

	/* SPM_SRC5_MASK */
	mmio_write_32(SPM_SRC5_MASK,
		((pwrctrl->reg_mcusys_merge_apsrc_req_mask_b & 0x1ff) << 0) |
		((pwrctrl->reg_mcusys_merge_ddren_req_mask_b & 0x1ff) << 9) |
		((pwrctrl->reg_afe_srcclkena_mask_b & 0x1) << 18) |
		((pwrctrl->reg_afe_infra_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_afe_apsrc_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_afe_vrf18_req_mask_b & 0x1) << 21) |
		((pwrctrl->reg_afe_ddren_req_mask_b & 0x1) << 22) |
		((pwrctrl->reg_msdc2_srcclkena_mask_b & 0x1) << 23) |
		((pwrctrl->reg_msdc2_infra_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_msdc2_apsrc_req_mask_b & 0x1) << 25) |
		((pwrctrl->reg_msdc2_vrf18_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_msdc2_ddren_req_mask_b & 0x1) << 27));

	/* SPM_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		((pwrctrl->reg_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_EXT_MASK,
		((pwrctrl->reg_ext_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_SRC7_MASK */
	mmio_write_32(SPM_SRC7_MASK,
		((pwrctrl->reg_pcie_srcclkena_mask_b & 0x1) << 0) |
		((pwrctrl->reg_pcie_infra_req_mask_b & 0x1) << 1) |
		((pwrctrl->reg_pcie_apsrc_req_mask_b & 0x1) << 2) |
		((pwrctrl->reg_pcie_vrf18_req_mask_b & 0x1) << 3) |
		((pwrctrl->reg_pcie_ddren_req_mask_b & 0x1) << 4) |
		((pwrctrl->reg_dpmaif_srcclkena_mask_b & 0x1) << 5) |
		((pwrctrl->reg_dpmaif_infra_req_mask_b & 0x1) << 6) |
		((pwrctrl->reg_dpmaif_apsrc_req_mask_b & 0x1) << 7) |
		((pwrctrl->reg_dpmaif_vrf18_req_mask_b & 0x1) << 8) |
		((pwrctrl->reg_dpmaif_ddren_req_mask_b & 0x1) << 9));
	/* Auto-gen End */
}

void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	uint32_t val, mask;

	/* toggle event counter clear */
	mmio_setbits_32(PCM_CON1,
			SPM_REGWR_CFG_KEY | SPM_EVENT_COUNTER_CLR_LSB);

	/* toggle for reset SYS TIMER start point */
	mmio_setbits_32(SYS_TIMER_CON, SYS_TIMER_START_EN_LSB);

	if (pwrctrl->timer_val_cust == 0U) {
		val = pwrctrl->timer_val ? (pwrctrl->timer_val) : (PCM_TIMER_MAX);
	} else {
		val = pwrctrl->timer_val_cust;
	}

	mmio_write_32(PCM_TIMER_VAL, val);
	mmio_setbits_32(PCM_CON1, (SPM_REGWR_CFG_KEY | RG_PCM_TIMER_EN_LSB));

	/* unmask AP wakeup source */
	if (pwrctrl->wake_src_cust == 0U) {
		mask = pwrctrl->wake_src;
	} else {
		mask = pwrctrl->wake_src_cust;
	}

	if (pwrctrl->reg_csyspwrup_ack_mask != 0U) {
		mask &= ~R12_CSYSPWREQ_B;
	}

	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~mask);

	/* unmask SPM ISR (keep TWAM setting) */
	mmio_setbits_32(SPM_IRQ_MASK, ISRM_RET_IRQ_AUX);

	/* toggle event counter clear */
	mmio_clrsetbits_32(PCM_CON1, SPM_EVENT_COUNTER_CLR_LSB,
			   SPM_REGWR_CFG_KEY);
	/* toggle for reset SYS TIMER start point */
	mmio_clrbits_32(SYS_TIMER_CON, SYS_TIMER_START_EN_LSB);
}

void __spm_ext_int_wakeup_req_clr(void)
{
	unsigned int reg = mmio_read_32(SPM_MD32_IRQ) & (~(0x1U << 0));

	mmio_write_32(EXT_INT_WAKEUP_REQ_CLR, mmio_read_32(ROOT_CPUTOP_ADDR));

	/* Clear spm2mcupm wakeup interrupt status */
	mmio_write_32(SPM_MD32_IRQ, reg);
}
