/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <common/debug.h>
#include <platform_def.h>
#include <mt_lp_rm.h>
#include <mt_lp_rqm.h>
#include <mt_lpm_dispatch.h>
#include <mt_lpm_smc.h>
#include <mt_spm_cond.h>
#include <mt_spm_dispatcher.h>
#include <mt_spm_internal.h>
#include <mt_spm_smc.h>
#include <mt_spm_constraint.h>
#include <mt_spm_conservation.h>
#include <mt_spm_suspend.h>
#include <mt_spm_idle.h>
#include <mt_spm_doe_resource_ctrl.h>

int contraint_update_status(unsigned int id, unsigned int act,
					unsigned int type, void *value)
{
	struct rc_common_state notify;

	notify.id = id;
	notify.act = act;
	notify.type = type;
	notify.value = value;

	return mt_lp_rm_do_update(-1, PLAT_RC_STATUS, &notify);
}

int contraint_do_valid(unsigned int id, unsigned int act,
			  unsigned int valid,
			  struct constraint_status *d_con,
			  unsigned int *result)
{
	if (!d_con || !result)
		return -1;

	if (act & MT_LPM_SMC_ACT_GET) {
		struct rc_common_state st;

		st.id = id;
		st.act = act;
		st.type = CONSTRAINT_GET_VALID;
		st.value = (void *)d_con;
		mt_lp_rm_get_status(PLAT_RC_STATUS, &st);
		*result = !!(d_con->is_valid & valid);
	} else {
		d_con->is_valid = (uint16_t)valid;
		contraint_update_status((unsigned int)id,
				(unsigned int)act,
				CONSTRAINT_UPDATE_VALID,
				(void *)d_con);
	}

	return 0;
}


/* code gen by spm_pwr_ctrl_atf.pl, need struct pwr_ctrl */
void save_pwr_ctrl(struct pwr_ctrl *pwrctrl,
			     unsigned int index,
			     unsigned int val)
{
	if (!pwrctrl)
		return;

	switch (index) {
	case PW_PCM_FLAGS:
		pwrctrl->pcm_flags = val;
		break;
	case PW_PCM_FLAGS_CUST:
		pwrctrl->pcm_flags_cust = val;
		break;
	case PW_PCM_FLAGS_CUST_SET:
		pwrctrl->pcm_flags_cust_set = val;
		break;
	case PW_PCM_FLAGS_CUST_CLR:
		pwrctrl->pcm_flags_cust_clr = val;
		break;
	case PW_PCM_FLAGS1:
		pwrctrl->pcm_flags1 = val;
		break;
	case PW_PCM_FLAGS1_CUST:
		pwrctrl->pcm_flags1_cust = val;
		break;
	case PW_PCM_FLAGS1_CUST_SET:
		pwrctrl->pcm_flags1_cust_set = val;
		break;
	case PW_PCM_FLAGS1_CUST_CLR:
		pwrctrl->pcm_flags1_cust_clr = val;
		break;
	case PW_TIMER_VAL:
		pwrctrl->timer_val = val;
		break;
	case PW_TIMER_VAL_CUST:
		pwrctrl->timer_val_cust = val;
		break;
	case PW_TIMER_VAL_RAMP_EN:
		pwrctrl->timer_val_ramp_en = val;
		break;
	case PW_TIMER_VAL_RAMP_EN_SEC:
		pwrctrl->timer_val_ramp_en_sec = val;
		break;
	case PW_WAKE_SRC:
		pwrctrl->wake_src = val;
		break;
	case PW_WAKE_SRC_CUST:
		pwrctrl->wake_src_cust = val;
		break;
	case PW_WAKELOCK_TIMER_VAL:
		pwrctrl->wakelock_timer_val = val;
		break;
	case PW_WDT_DISABLE:
		pwrctrl->wdt_disable = val;
		break;
	case PW_REG_SRCCLKEN0_CTL:
		pwrctrl->reg_srcclken0_ctl = val;
		break;
	case PW_REG_SRCCLKEN1_CTL:
		pwrctrl->reg_srcclken1_ctl = val;
		break;
	case PW_REG_SPM_LOCK_INFRA_DCM:
		pwrctrl->reg_spm_lock_infra_dcm = val;
		break;
	case PW_REG_SRCCLKEN_MASK:
		pwrctrl->reg_srcclken_mask = val;
		break;
	case PW_REG_MD1_C32RM_EN:
		pwrctrl->reg_md1_c32rm_en = val;
		break;
	case PW_REG_MD2_C32RM_EN:
		pwrctrl->reg_md2_c32rm_en = val;
		break;
	case PW_REG_CLKSQ0_SEL_CTRL:
		pwrctrl->reg_clksq0_sel_ctrl = val;
		break;
	case PW_REG_CLKSQ1_SEL_CTRL:
		pwrctrl->reg_clksq1_sel_ctrl = val;
		break;
	case PW_REG_SRCCLKEN0_EN:
		pwrctrl->reg_srcclken0_en = val;
		break;
	case PW_REG_SRCCLKEN1_EN:
		pwrctrl->reg_srcclken1_en = val;
		break;
	case PW_REG_SYSCLK0_SRC_MASK_B:
		pwrctrl->reg_sysclk0_src_mask_b = val;
		break;
	case PW_REG_SYSCLK1_SRC_MASK_B:
		pwrctrl->reg_sysclk1_src_mask_b = val;
		break;
	/* SPM_AP_STANDBY_CON */
	case PW_REG_WFI_OP:
		pwrctrl->reg_wfi_op = val;
		break;
	case PW_REG_WFI_TYPE:
		pwrctrl->reg_wfi_type = val;
		break;
	case PW_REG_MP0_CPUTOP_IDLE_MASK:
		pwrctrl->reg_mp0_cputop_idle_mask = val;
		break;
	case PW_REG_MP1_CPUTOP_IDLE_MASK:
		pwrctrl->reg_mp1_cputop_idle_mask = val;
		break;
	case PW_REG_MCUSYS_IDLE_MASK:
		pwrctrl->reg_mcusys_idle_mask = val;
		break;
	case PW_REG_MD_APSRC_1_SEL:
		pwrctrl->reg_md_apsrc_1_sel = val;
		break;
	case PW_REG_MD_APSRC_0_SEL:
		pwrctrl->reg_md_apsrc_0_sel = val;
		break;
	case PW_REG_CONN_APSRC_SEL:
		pwrctrl->reg_conn_apsrc_sel = val;
		break;
	/* SPM_SRC6_MASK */
	case PW_REG_DPMAIF_SRCCLKENA_MASK_B:
		pwrctrl->reg_dpmaif_srcclkena_mask_b = val;
		break;
	case PW_REG_DPMAIF_INFRA_REQ_MASK_B:
		pwrctrl->reg_dpmaif_infra_req_mask_b = val;
		break;
	case PW_REG_DPMAIF_APSRC_REQ_MASK_B:
		pwrctrl->reg_dpmaif_apsrc_req_mask_b = val;
		break;
	case PW_REG_DPMAIF_VRF18_REQ_MASK_B:
		pwrctrl->reg_dpmaif_vrf18_req_mask_b = val;
		break;
	case PW_REG_DPMAIF_DDR_EN_MASK_B:
		pwrctrl->reg_dpmaif_ddr_en_mask_b = val;
		break;

	/* SPM_SRC_REQ */
	case PW_REG_SPM_APSRC_REQ:
		pwrctrl->reg_spm_apsrc_req = val;
		break;
	case PW_REG_SPM_F26M_REQ:
		pwrctrl->reg_spm_f26m_req = val;
		break;
	case PW_REG_SPM_INFRA_REQ:
		pwrctrl->reg_spm_infra_req = val;
		break;
	case PW_REG_SPM_VRF18_REQ:
		pwrctrl->reg_spm_vrf18_req = val;
		break;
	case PW_REG_SPM_DDR_EN_REQ:
		pwrctrl->reg_spm_ddr_en_req = val;
		break;
	case PW_REG_SPM_DVFS_REQ:
		pwrctrl->reg_spm_dvfs_req = val;
		break;
	case PW_REG_SPM_SW_MAILBOX_REQ:
		pwrctrl->reg_spm_sw_mailbox_req = val;
		break;
	case PW_REG_SPM_SSPM_MAILBOX_REQ:
		pwrctrl->reg_spm_sspm_mailbox_req = val;
		break;
	case PW_REG_SPM_ADSP_MAILBOX_REQ:
		pwrctrl->reg_spm_adsp_mailbox_req = val;
		break;
	case PW_REG_SPM_SCP_MAILBOX_REQ:
		pwrctrl->reg_spm_scp_mailbox_req = val;
		break;

	/* SPM_SRC_MASK */
	case PW_REG_MD_SRCCLKENA_0_MASK_B:
		pwrctrl->reg_md_srcclkena_0_mask_b = val;
		break;
	case PW_REG_MD_SRCCLKENA2INFRA_REQ_0_MASK_B:
		pwrctrl->reg_md_srcclkena2infra_req_0_mask_b = val;
		break;
	case PW_REG_MD_APSRC2INFRA_REQ_0_MASK_B:
		pwrctrl->reg_md_apsrc2infra_req_0_mask_b = val;
		break;
	case PW_REG_MD_APSRC_REQ_0_MASK_B:
		pwrctrl->reg_md_apsrc_req_0_mask_b = val;
		break;
	case PW_REG_MD_VRF18_REQ_0_MASK_B:
		pwrctrl->reg_md_vrf18_req_0_mask_b = val;
		break;
	case PW_REG_MD_DDR_EN_0_MASK_B:
		pwrctrl->reg_md_ddr_en_0_mask_b = val;
		break;
	case PW_REG_MD_SRCCLKENA_1_MASK_B:
		pwrctrl->reg_md_srcclkena_1_mask_b = val;
		break;
	case PW_REG_MD_SRCCLKENA2INFRA_REQ_1_MASK_B:
		pwrctrl->reg_md_srcclkena2infra_req_1_mask_b = val;
		break;
	case PW_REG_MD_APSRC2INFRA_REQ_1_MASK_B:
		pwrctrl->reg_md_apsrc2infra_req_1_mask_b = val;
		break;
	case PW_REG_MD_APSRC_REQ_1_MASK_B:
		pwrctrl->reg_md_apsrc_req_1_mask_b = val;
		break;
	case PW_REG_MD_VRF18_REQ_1_MASK_B:
		pwrctrl->reg_md_vrf18_req_1_mask_b = val;
		break;
	case PW_REG_MD_DDR_EN_1_MASK_B:
		pwrctrl->reg_md_ddr_en_1_mask_b = val;
		break;
	case PW_REG_CONN_SRCCLKENA_MASK_B:
		pwrctrl->reg_conn_srcclkena_mask_b = val;
		break;
	case PW_REG_CONN_SRCCLKENB_MASK_B:
		pwrctrl->reg_conn_srcclkenb_mask_b = val;
		break;
	case PW_REG_CONN_INFRA_REQ_MASK_B:
		pwrctrl->reg_conn_infra_req_mask_b = val;
		break;
	case PW_REG_CONN_APSRC_REQ_MASK_B:
		pwrctrl->reg_conn_apsrc_req_mask_b = val;
		break;
	case PW_REG_CONN_VRF18_REQ_MASK_B:
		pwrctrl->reg_conn_vrf18_req_mask_b = val;
		break;
	case PW_REG_CONN_DDR_EN_MASK_B:
		pwrctrl->reg_conn_ddr_en_mask_b = val;
		break;
	case PW_REG_CONN_VFE28_MASK_B:
		pwrctrl->reg_conn_vfe28_mask_b = val;
		break;
	case PW_REG_SRCCLKENI0_SRCCLKENA_MASK_B:
		pwrctrl->reg_srcclkeni0_srcclkena_mask_b = val;
		break;
	case PW_REG_SRCCLKENI0_INFRA_REQ_MASK_B:
		pwrctrl->reg_srcclkeni0_infra_req_mask_b = val;
		break;
	case PW_REG_SRCCLKENI1_SRCCLKENA_MASK_B:
		pwrctrl->reg_srcclkeni1_srcclkena_mask_b = val;
		break;
	case PW_REG_SRCCLKENI1_INFRA_REQ_MASK_B:
		pwrctrl->reg_srcclkeni1_infra_req_mask_b = val;
		break;
	case PW_REG_SRCCLKENI2_SRCCLKENA_MASK_B:
		pwrctrl->reg_srcclkeni2_srcclkena_mask_b = val;
		break;
	case PW_REG_SRCCLKENI2_INFRA_REQ_MASK_B:
		pwrctrl->reg_srcclkeni2_infra_req_mask_b = val;
		break;
	case PW_REG_INFRASYS_APSRC_REQ_MASK_B:
		pwrctrl->reg_infrasys_apsrc_req_mask_b = val;
		break;
	case PW_REG_INFRASYS_DDR_EN_MASK_B:
		pwrctrl->reg_infrasys_ddr_en_mask_b = val;
		break;
	case PW_REG_MD32_SRCCLKENA_MASK_B:
		pwrctrl->reg_md32_srcclkena_mask_b = val;
		break;
	case PW_REG_MD32_INFRA_REQ_MASK_B:
		pwrctrl->reg_md32_infra_req_mask_b = val;
		break;
	case PW_REG_MD32_APSRC_REQ_MASK_B:
		pwrctrl->reg_md32_apsrc_req_mask_b = val;
		break;
	case PW_REG_MD32_VRF18_REQ_MASK_B:
		pwrctrl->reg_md32_vrf18_req_mask_b = val;
		break;
	case PW_REG_MD32_DDR_EN_MASK_B:
		pwrctrl->reg_md32_ddr_en_mask_b = val;
		break;

	/* SPM_SRC2_MASK */
	case PW_REG_SCP_SRCCLKENA_MASK_B:
		pwrctrl->reg_scp_srcclkena_mask_b = val;
		break;
	case PW_REG_SCP_INFRA_REQ_MASK_B:
		pwrctrl->reg_scp_infra_req_mask_b = val;
		break;
	case PW_REG_SCP_APSRC_REQ_MASK_B:
		pwrctrl->reg_scp_apsrc_req_mask_b = val;
		break;
	case PW_REG_SCP_VRF18_REQ_MASK_B:
		pwrctrl->reg_scp_vrf18_req_mask_b = val;
		break;
	case PW_REG_SCP_DDR_EN_MASK_B:
		pwrctrl->reg_scp_ddr_en_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_SRCCLKENA_MASK_B:
		pwrctrl->reg_audio_dsp_srcclkena_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_INFRA_REQ_MASK_B:
		pwrctrl->reg_audio_dsp_infra_req_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_APSRC_REQ_MASK_B:
		pwrctrl->reg_audio_dsp_apsrc_req_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_VRF18_REQ_MASK_B:
		pwrctrl->reg_audio_dsp_vrf18_req_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_DDR_EN_MASK_B:
		pwrctrl->reg_audio_dsp_ddr_en_mask_b = val;
		break;
	case PW_REG_UFS_SRCCLKENA_MASK_B:
		pwrctrl->reg_ufs_srcclkena_mask_b = val;
		break;
	case PW_REG_UFS_INFRA_REQ_MASK_B:
		pwrctrl->reg_ufs_infra_req_mask_b = val;
		break;
	case PW_REG_UFS_APSRC_REQ_MASK_B:
		pwrctrl->reg_ufs_apsrc_req_mask_b = val;
		break;
	case PW_REG_UFS_VRF18_REQ_MASK_B:
		pwrctrl->reg_ufs_vrf18_req_mask_b = val;
		break;
	case PW_REG_UFS_DDR_EN_MASK_B:
		pwrctrl->reg_ufs_ddr_en_mask_b = val;
		break;
	case PW_REG_DISP0_APSRC_REQ_MASK_B:
		pwrctrl->reg_disp0_apsrc_req_mask_b = val;
		break;
	case PW_REG_DISP0_DDR_EN_MASK_B:
		pwrctrl->reg_disp0_ddr_en_mask_b = val;
		break;
	case PW_REG_DISP1_APSRC_REQ_MASK_B:
		pwrctrl->reg_disp1_apsrc_req_mask_b = val;
		break;
	case PW_REG_DISP1_DDR_EN_MASK_B:
		pwrctrl->reg_disp1_ddr_en_mask_b = val;
		break;
	case PW_REG_GCE_INFRA_REQ_MASK_B:
		pwrctrl->reg_gce_infra_req_mask_b = val;
		break;
	case PW_REG_GCE_APSRC_REQ_MASK_B:
		pwrctrl->reg_gce_apsrc_req_mask_b = val;
		break;
	case PW_REG_GCE_VRF18_REQ_MASK_B:
		pwrctrl->reg_gce_vrf18_req_mask_b = val;
		break;
	case PW_REG_GCE_DDR_EN_MASK_B:
		pwrctrl->reg_gce_ddr_en_mask_b = val;
		break;
	case PW_REG_APU_SRCCLKENA_MASK_B:
		pwrctrl->reg_apu_srcclkena_mask_b = val;
		break;
	case PW_REG_APU_INFRA_REQ_MASK_B:
		pwrctrl->reg_apu_infra_req_mask_b = val;
		break;
	case PW_REG_APU_APSRC_REQ_MASK_B:
		pwrctrl->reg_apu_apsrc_req_mask_b = val;
		break;
	case PW_REG_APU_VRF18_REQ_MASK_B:
		pwrctrl->reg_apu_vrf18_req_mask_b = val;
		break;
	case PW_REG_APU_DDR_EN_MASK_B:
		pwrctrl->reg_apu_ddr_en_mask_b = val;
		break;
	case PW_REG_CG_CHECK_SRCCLKENA_MASK_B:
		pwrctrl->reg_cg_check_srcclkena_mask_b = val;
		break;
	case PW_REG_CG_CHECK_APSRC_REQ_MASK_B:
		pwrctrl->reg_cg_check_apsrc_req_mask_b = val;
		break;
	case PW_REG_CG_CHECK_VRF18_REQ_MASK_B:
		pwrctrl->reg_cg_check_vrf18_req_mask_b = val;
		break;
	case PW_REG_CG_CHECK_DDR_EN_MASK_B:
		pwrctrl->reg_cg_check_ddr_en_mask_b = val;
		break;

	/* SPM_SRC3_MASK */
	case PW_REG_DVFSRC_EVENT_TRIGGER_MASK_B:
		pwrctrl->reg_dvfsrc_event_trigger_mask_b = val;
		break;
	case PW_REG_SW2SPM_INT0_MASK_B:
		pwrctrl->reg_sw2spm_int0_mask_b = val;
		break;
	case PW_REG_SW2SPM_INT1_MASK_B:
		pwrctrl->reg_sw2spm_int1_mask_b = val;
		break;
	case PW_REG_SW2SPM_INT2_MASK_B:
		pwrctrl->reg_sw2spm_int2_mask_b = val;
		break;
	case PW_REG_SW2SPM_INT3_MASK_B:
		pwrctrl->reg_sw2spm_int3_mask_b = val;
		break;
	case PW_REG_SC_ADSP2SPM_WAKEUP_MASK_B:
		pwrctrl->reg_sc_adsp2spm_wakeup_mask_b = val;
		break;
	case PW_REG_SC_SSPM2SPM_WAKEUP_MASK_B:
		pwrctrl->reg_sc_sspm2spm_wakeup_mask_b = val;
		break;
	case PW_REG_SC_SCP2SPM_WAKEUP_MASK_B:
		pwrctrl->reg_sc_scp2spm_wakeup_mask_b = val;
		break;
	case PW_REG_CSYSPWRREQ_MASK:
		pwrctrl->reg_csyspwrreq_mask = val;
		break;
	case PW_REG_SPM_SRCCLKENA_RESERVED_MASK_B:
		pwrctrl->reg_spm_srcclkena_reserved_mask_b = val;
		break;
	case PW_REG_SPM_INFRA_REQ_RESERVED_MASK_B:
		pwrctrl->reg_spm_infra_req_reserved_mask_b = val;
		break;
	case PW_REG_SPM_APSRC_REQ_RESERVED_MASK_B:
		pwrctrl->reg_spm_apsrc_req_reserved_mask_b = val;
		break;
	case PW_REG_SPM_VRF18_REQ_RESERVED_MASK_B:
		pwrctrl->reg_spm_vrf18_req_reserved_mask_b = val;
		break;
	case PW_REG_SPM_DDR_EN_RESERVED_MASK_B:
		pwrctrl->reg_spm_ddr_en_reserved_mask_b = val;
		break;
	case PW_REG_MCUPM_SRCCLKENA_MASK_B:
		pwrctrl->reg_mcupm_srcclkena_mask_b = val;
		break;
	case PW_REG_MCUPM_INFRA_REQ_MASK_B:
		pwrctrl->reg_mcupm_infra_req_mask_b = val;
		break;
	case PW_REG_MCUPM_APSRC_REQ_MASK_B:
		pwrctrl->reg_mcupm_apsrc_req_mask_b = val;
		break;
	case PW_REG_MCUPM_VRF18_REQ_MASK_B:
		pwrctrl->reg_mcupm_vrf18_req_mask_b = val;
		break;
	case PW_REG_MCUPM_DDR_EN_MASK_B:
		pwrctrl->reg_mcupm_ddr_en_mask_b = val;
		break;
	case PW_REG_MSDC0_SRCCLKENA_MASK_B:
		pwrctrl->reg_msdc0_srcclkena_mask_b = val;
		break;
	case PW_REG_MSDC0_INFRA_REQ_MASK_B:
		pwrctrl->reg_msdc0_infra_req_mask_b = val;
		break;
	case PW_REG_MSDC0_APSRC_REQ_MASK_B:
		pwrctrl->reg_msdc0_apsrc_req_mask_b = val;
		break;
	case PW_REG_MSDC0_VRF18_REQ_MASK_B:
		pwrctrl->reg_msdc0_vrf18_req_mask_b = val;
		break;
	case PW_REG_MSDC0_DDR_EN_MASK_B:
		pwrctrl->reg_msdc0_ddr_en_mask_b = val;
		break;
	case PW_REG_MSDC1_SRCCLKENA_MASK_B:
		pwrctrl->reg_msdc1_srcclkena_mask_b = val;
		break;
	case PW_REG_MSDC1_INFRA_REQ_MASK_B:
		pwrctrl->reg_msdc1_infra_req_mask_b = val;
		break;
	case PW_REG_MSDC1_APSRC_REQ_MASK_B:
		pwrctrl->reg_msdc1_apsrc_req_mask_b = val;
		break;
	case PW_REG_MSDC1_VRF18_REQ_MASK_B:
		pwrctrl->reg_msdc1_vrf18_req_mask_b = val;
		break;
	case PW_REG_MSDC1_DDR_EN_MASK_B:
		pwrctrl->reg_msdc1_ddr_en_mask_b = val;
		break;

	/* SPM_SRC4_MASK */
	case PW_CCIF_EVENT_MASK_B:
		pwrctrl->ccif_event_mask_b = val;
		break;
	case PW_REG_BAK_PSRI_SRCCLKENA_MASK_B:
		pwrctrl->reg_bak_psri_srcclkena_mask_b = val;
		break;
	case PW_REG_BAK_PSRI_INFRA_REQ_MASK_B:
		pwrctrl->reg_bak_psri_infra_req_mask_b = val;
		break;
	case PW_REG_BAK_PSRI_APSRC_REQ_MASK_B:
		pwrctrl->reg_bak_psri_apsrc_req_mask_b = val;
		break;
	case PW_REG_BAK_PSRI_VRF18_REQ_MASK_B:
		pwrctrl->reg_bak_psri_vrf18_req_mask_b = val;
		break;
	case PW_REG_BAK_PSRI_DDR_EN_MASK_B:
		pwrctrl->reg_bak_psri_ddr_en_mask_b = val;
		break;
	case PW_REG_DRAMC0_MD32_INFRA_REQ_MASK_B:
		pwrctrl->reg_dramc0_md32_infra_req_mask_b = val;
		break;
	case PW_REG_DRAMC0_MD32_VRF18_REQ_MASK_B:
		pwrctrl->reg_dramc0_md32_vrf18_req_mask_b = val;
		break;
	case PW_REG_DRAMC1_MD32_INFRA_REQ_MASK_B:
		pwrctrl->reg_dramc1_md32_infra_req_mask_b = val;
		break;
	case PW_REG_DRAMC1_MD32_VRF18_REQ_MASK_B:
		pwrctrl->reg_dramc1_md32_vrf18_req_mask_b = val;
		break;
	case PW_REG_CONN_SRCCLKENB2PWRAP_MASK_B:
		pwrctrl->reg_conn_srcclkenb2pwrap_mask_b = val;
		break;
	case PW_REG_DRAMC0_MD32_WAKEUP_MASK:
		pwrctrl->reg_dramc0_md32_wakeup_mask = val;
		break;
	case PW_REG_DRAMC1_MD32_WAKEUP_MASK:
		pwrctrl->reg_dramc1_md32_wakeup_mask = val;
		break;

	/* SPM_SRC5_MASK */
	case PW_REG_MCUSYS_MERGE_APSRC_REQ_MASK_B:
		pwrctrl->reg_mcusys_merge_apsrc_req_mask_b = val;
		break;
	case PW_REG_MCUSYS_MERGE_DDR_EN_MASK_B:
		pwrctrl->reg_mcusys_merge_ddr_en_mask_b = val;
		break;
	case PW_REG_MSDC2_SRCCLKENA_MASK_B:
		pwrctrl->reg_msdc2_srcclkena_mask_b = val;
		break;
	case PW_REG_MSDC2_INFRA_REQ_MASK_B:
		pwrctrl->reg_msdc2_infra_req_mask_b = val;
		break;
	case PW_REG_MSDC2_APSRC_REQ_MASK_B:
		pwrctrl->reg_msdc2_apsrc_req_mask_b = val;
		break;
	case PW_REG_MSDC2_VRF18_REQ_MASK_B:
		pwrctrl->reg_msdc2_vrf18_req_mask_b = val;
		break;
	case PW_REG_MSDC2_DDR_EN_MASK_B:
		pwrctrl->reg_msdc2_ddr_en_mask_b = val;
		break;
	case PW_REG_PCIE_SRCCLKENA_MASK_B:
		pwrctrl->reg_pcie_srcclkena_mask_b = val;
		break;
	case PW_REG_PCIE_INFRA_REQ_MASK_B:
		pwrctrl->reg_pcie_infra_req_mask_b = val;
		break;
	case PW_REG_PCIE_APSRC_REQ_MASK_B:
		pwrctrl->reg_pcie_apsrc_req_mask_b = val;
		break;
	case PW_REG_PCIE_VRF18_REQ_MASK_B:
		pwrctrl->reg_pcie_vrf18_req_mask_b = val;
		break;
	case PW_REG_PCIE_DDR_EN_MASK_B:
		pwrctrl->reg_pcie_ddr_en_mask_b = val;
		break;

	/* SPM_WAKEUP_EVENT_MASK */
	case PW_REG_WAKEUP_EVENT_MASK:
		pwrctrl->reg_wakeup_event_mask = val;
		break;

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	case PW_REG_EXT_WAKEUP_EVENT_MASK:
		pwrctrl->reg_ext_wakeup_event_mask = val;
		break;
	default:
		ERROR("%s: %d is undefined!!!\n", __func__, index);
	}
}

/* code gen by spm_pwr_ctrl_atf.pl, need struct pwr_ctrl */
unsigned int load_pwr_ctrl(struct pwr_ctrl *pwrctrl, unsigned int index)
{
	unsigned int val = 0;

	if (!pwrctrl)
		return 0;

	switch (index) {
	case PW_PCM_FLAGS:
		val = pwrctrl->pcm_flags;
		break;
	case PW_PCM_FLAGS_CUST:
		val = pwrctrl->pcm_flags_cust;
		break;
	case PW_PCM_FLAGS_CUST_SET:
		val = pwrctrl->pcm_flags_cust_set;
		break;
	case PW_PCM_FLAGS_CUST_CLR:
		val = pwrctrl->pcm_flags_cust_clr;
		break;
	case PW_PCM_FLAGS1:
		val = pwrctrl->pcm_flags1;
		break;
	case PW_PCM_FLAGS1_CUST:
		val = pwrctrl->pcm_flags1_cust;
		break;
	case PW_PCM_FLAGS1_CUST_SET:
		val = pwrctrl->pcm_flags1_cust_set;
		break;
	case PW_PCM_FLAGS1_CUST_CLR:
		val = pwrctrl->pcm_flags1_cust_clr;
		break;
	case PW_TIMER_VAL:
		val = pwrctrl->timer_val;
		break;
	case PW_TIMER_VAL_CUST:
		val = pwrctrl->timer_val_cust;
		break;
	case PW_TIMER_VAL_RAMP_EN:
		val = pwrctrl->timer_val_ramp_en;
		break;
	case PW_TIMER_VAL_RAMP_EN_SEC:
		val = pwrctrl->timer_val_ramp_en_sec;
		break;
	case PW_WAKE_SRC:
		val = pwrctrl->wake_src;
		break;
	case PW_WAKE_SRC_CUST:
		val = pwrctrl->wake_src_cust;
		break;
	case PW_WAKELOCK_TIMER_VAL:
		val = pwrctrl->wakelock_timer_val;
		break;
	case PW_WDT_DISABLE:
		val = pwrctrl->wdt_disable;
		break;
	case PW_REG_SRCCLKEN0_CTL:
		val = pwrctrl->reg_srcclken0_ctl;
		break;
	case PW_REG_SRCCLKEN1_CTL:
		val = pwrctrl->reg_srcclken1_ctl;
		break;
	case PW_REG_SPM_LOCK_INFRA_DCM:
		val = pwrctrl->reg_spm_lock_infra_dcm;
		break;
	case PW_REG_SRCCLKEN_MASK:
		val = pwrctrl->reg_srcclken_mask;
		break;
	case PW_REG_MD1_C32RM_EN:
		val = pwrctrl->reg_md1_c32rm_en;
		break;
	case PW_REG_MD2_C32RM_EN:
		val = pwrctrl->reg_md2_c32rm_en;
		break;
	case PW_REG_CLKSQ0_SEL_CTRL:
		val = pwrctrl->reg_clksq0_sel_ctrl;
		break;
	case PW_REG_CLKSQ1_SEL_CTRL:
		val = pwrctrl->reg_clksq1_sel_ctrl;
		break;
	case PW_REG_SRCCLKEN0_EN:
		val = pwrctrl->reg_srcclken0_en;
		break;
	case PW_REG_SRCCLKEN1_EN:
		val = pwrctrl->reg_srcclken1_en;
		break;
	case PW_REG_SYSCLK0_SRC_MASK_B:
		val = pwrctrl->reg_sysclk0_src_mask_b;
		break;
	case PW_REG_SYSCLK1_SRC_MASK_B:
		val = pwrctrl->reg_sysclk1_src_mask_b;
		break;
	/* SPM_AP_STANDBY_CON */
	case PW_REG_WFI_OP:
		val = pwrctrl->reg_wfi_op;
		break;
	case PW_REG_WFI_TYPE:
		val = pwrctrl->reg_wfi_type;
		break;
	case PW_REG_MP0_CPUTOP_IDLE_MASK:
		val = pwrctrl->reg_mp0_cputop_idle_mask;
		break;
	case PW_REG_MP1_CPUTOP_IDLE_MASK:
		val = pwrctrl->reg_mp1_cputop_idle_mask;
		break;
	case PW_REG_MCUSYS_IDLE_MASK:
		val = pwrctrl->reg_mcusys_idle_mask;
		break;
	case PW_REG_MD_APSRC_1_SEL:
		val = pwrctrl->reg_md_apsrc_1_sel;
		break;
	case PW_REG_MD_APSRC_0_SEL:
		val = pwrctrl->reg_md_apsrc_0_sel;
		break;
	case PW_REG_CONN_APSRC_SEL:
		val = pwrctrl->reg_conn_apsrc_sel;
		break;

	/* SPM_SRC6_MASK */
	case PW_REG_DPMAIF_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_dpmaif_srcclkena_mask_b;
		break;
	case PW_REG_DPMAIF_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_dpmaif_infra_req_mask_b;
		break;
	case PW_REG_DPMAIF_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_dpmaif_apsrc_req_mask_b;
		break;
	case PW_REG_DPMAIF_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_dpmaif_vrf18_req_mask_b;
		break;
	case PW_REG_DPMAIF_DDR_EN_MASK_B:
		val = pwrctrl->reg_dpmaif_ddr_en_mask_b;
		break;

	/* SPM_SRC_REQ */
	case PW_REG_SPM_APSRC_REQ:
		val = pwrctrl->reg_spm_apsrc_req;
		break;
	case PW_REG_SPM_F26M_REQ:
		val = pwrctrl->reg_spm_f26m_req;
		break;
	case PW_REG_SPM_INFRA_REQ:
		val = pwrctrl->reg_spm_infra_req;
		break;
	case PW_REG_SPM_VRF18_REQ:
		val = pwrctrl->reg_spm_vrf18_req;
		break;
	case PW_REG_SPM_DDR_EN_REQ:
		val = pwrctrl->reg_spm_ddr_en_req;
		break;
	case PW_REG_SPM_DVFS_REQ:
		val = pwrctrl->reg_spm_dvfs_req;
		break;
	case PW_REG_SPM_SW_MAILBOX_REQ:
		val = pwrctrl->reg_spm_sw_mailbox_req;
		break;
	case PW_REG_SPM_SSPM_MAILBOX_REQ:
		val = pwrctrl->reg_spm_sspm_mailbox_req;
		break;
	case PW_REG_SPM_ADSP_MAILBOX_REQ:
		val = pwrctrl->reg_spm_adsp_mailbox_req;
		break;
	case PW_REG_SPM_SCP_MAILBOX_REQ:
		val = pwrctrl->reg_spm_scp_mailbox_req;
		break;

	/* SPM_SRC_MASK */
	case PW_REG_MD_SRCCLKENA_0_MASK_B:
		val = pwrctrl->reg_md_srcclkena_0_mask_b;
		break;
	case PW_REG_MD_SRCCLKENA2INFRA_REQ_0_MASK_B:
		val = pwrctrl->reg_md_srcclkena2infra_req_0_mask_b;
		break;
	case PW_REG_MD_APSRC2INFRA_REQ_0_MASK_B:
		val = pwrctrl->reg_md_apsrc2infra_req_0_mask_b;
		break;
	case PW_REG_MD_APSRC_REQ_0_MASK_B:
		val = pwrctrl->reg_md_apsrc_req_0_mask_b;
		break;
	case PW_REG_MD_VRF18_REQ_0_MASK_B:
		val = pwrctrl->reg_md_vrf18_req_0_mask_b;
		break;
	case PW_REG_MD_DDR_EN_0_MASK_B:
		val = pwrctrl->reg_md_ddr_en_0_mask_b;
		break;
	case PW_REG_MD_SRCCLKENA_1_MASK_B:
		val = pwrctrl->reg_md_srcclkena_1_mask_b;
		break;
	case PW_REG_MD_SRCCLKENA2INFRA_REQ_1_MASK_B:
		val = pwrctrl->reg_md_srcclkena2infra_req_1_mask_b;
		break;
	case PW_REG_MD_APSRC2INFRA_REQ_1_MASK_B:
		val = pwrctrl->reg_md_apsrc2infra_req_1_mask_b;
		break;
	case PW_REG_MD_APSRC_REQ_1_MASK_B:
		val = pwrctrl->reg_md_apsrc_req_1_mask_b;
		break;
	case PW_REG_MD_VRF18_REQ_1_MASK_B:
		val = pwrctrl->reg_md_vrf18_req_1_mask_b;
		break;
	case PW_REG_MD_DDR_EN_1_MASK_B:
		val = pwrctrl->reg_md_ddr_en_1_mask_b;
		break;
	case PW_REG_CONN_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_conn_srcclkena_mask_b;
		break;
	case PW_REG_CONN_SRCCLKENB_MASK_B:
		val = pwrctrl->reg_conn_srcclkenb_mask_b;
		break;
	case PW_REG_CONN_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_conn_infra_req_mask_b;
		break;
	case PW_REG_CONN_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_conn_apsrc_req_mask_b;
		break;
	case PW_REG_CONN_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_conn_vrf18_req_mask_b;
		break;
	case PW_REG_CONN_DDR_EN_MASK_B:
		val = pwrctrl->reg_conn_ddr_en_mask_b;
		break;
	case PW_REG_CONN_VFE28_MASK_B:
		val = pwrctrl->reg_conn_vfe28_mask_b;
		break;
	case PW_REG_SRCCLKENI0_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_srcclkeni0_srcclkena_mask_b;
		break;
	case PW_REG_SRCCLKENI0_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_srcclkeni0_infra_req_mask_b;
		break;
	case PW_REG_SRCCLKENI1_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_srcclkeni1_srcclkena_mask_b;
		break;
	case PW_REG_SRCCLKENI1_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_srcclkeni1_infra_req_mask_b;
		break;
	case PW_REG_SRCCLKENI2_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_srcclkeni2_srcclkena_mask_b;
		break;
	case PW_REG_SRCCLKENI2_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_srcclkeni2_infra_req_mask_b;
		break;
	case PW_REG_INFRASYS_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_infrasys_apsrc_req_mask_b;
		break;
	case PW_REG_INFRASYS_DDR_EN_MASK_B:
		val = pwrctrl->reg_infrasys_ddr_en_mask_b;
		break;
	case PW_REG_MD32_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_md32_srcclkena_mask_b;
		break;
	case PW_REG_MD32_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_md32_infra_req_mask_b;
		break;
	case PW_REG_MD32_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_md32_apsrc_req_mask_b;
		break;
	case PW_REG_MD32_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_md32_vrf18_req_mask_b;
		break;
	case PW_REG_MD32_DDR_EN_MASK_B:
		val = pwrctrl->reg_md32_ddr_en_mask_b;
		break;

	/* SPM_SRC2_MASK */
	case PW_REG_SCP_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_scp_srcclkena_mask_b;
		break;
	case PW_REG_SCP_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_scp_infra_req_mask_b;
		break;
	case PW_REG_SCP_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_scp_apsrc_req_mask_b;
		break;
	case PW_REG_SCP_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_scp_vrf18_req_mask_b;
		break;
	case PW_REG_SCP_DDR_EN_MASK_B:
		val = pwrctrl->reg_scp_ddr_en_mask_b;
		break;
	case PW_REG_AUDIO_DSP_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_audio_dsp_srcclkena_mask_b;
		break;
	case PW_REG_AUDIO_DSP_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_audio_dsp_infra_req_mask_b;
		break;
	case PW_REG_AUDIO_DSP_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_audio_dsp_apsrc_req_mask_b;
		break;
	case PW_REG_AUDIO_DSP_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_audio_dsp_vrf18_req_mask_b;
		break;
	case PW_REG_AUDIO_DSP_DDR_EN_MASK_B:
		val = pwrctrl->reg_audio_dsp_ddr_en_mask_b;
		break;
	case PW_REG_UFS_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_ufs_srcclkena_mask_b;
		break;
	case PW_REG_UFS_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_ufs_infra_req_mask_b;
		break;
	case PW_REG_UFS_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_ufs_apsrc_req_mask_b;
		break;
	case PW_REG_UFS_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_ufs_vrf18_req_mask_b;
		break;
	case PW_REG_UFS_DDR_EN_MASK_B:
		val = pwrctrl->reg_ufs_ddr_en_mask_b;
		break;
	case PW_REG_DISP0_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_disp0_apsrc_req_mask_b;
		break;
	case PW_REG_DISP0_DDR_EN_MASK_B:
		val = pwrctrl->reg_disp0_ddr_en_mask_b;
		break;
	case PW_REG_DISP1_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_disp1_apsrc_req_mask_b;
		break;
	case PW_REG_DISP1_DDR_EN_MASK_B:
		val = pwrctrl->reg_disp1_ddr_en_mask_b;
		break;
	case PW_REG_GCE_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_gce_infra_req_mask_b;
		break;
	case PW_REG_GCE_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_gce_apsrc_req_mask_b;
		break;
	case PW_REG_GCE_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_gce_vrf18_req_mask_b;
		break;
	case PW_REG_GCE_DDR_EN_MASK_B:
		val = pwrctrl->reg_gce_ddr_en_mask_b;
		break;
	case PW_REG_APU_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_apu_srcclkena_mask_b;
		break;
	case PW_REG_APU_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_apu_infra_req_mask_b;
		break;
	case PW_REG_APU_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_apu_apsrc_req_mask_b;
		break;
	case PW_REG_APU_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_apu_vrf18_req_mask_b;
		break;
	case PW_REG_APU_DDR_EN_MASK_B:
		val = pwrctrl->reg_apu_ddr_en_mask_b;
		break;
	case PW_REG_CG_CHECK_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_cg_check_srcclkena_mask_b;
		break;
	case PW_REG_CG_CHECK_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_cg_check_apsrc_req_mask_b;
		break;
	case PW_REG_CG_CHECK_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_cg_check_vrf18_req_mask_b;
		break;
	case PW_REG_CG_CHECK_DDR_EN_MASK_B:
		val = pwrctrl->reg_cg_check_ddr_en_mask_b;
		break;

	/* SPM_SRC3_MASK */
	case PW_REG_DVFSRC_EVENT_TRIGGER_MASK_B:
		val = pwrctrl->reg_dvfsrc_event_trigger_mask_b;
		break;
	case PW_REG_SW2SPM_INT0_MASK_B:
		val = pwrctrl->reg_sw2spm_int0_mask_b;
		break;
	case PW_REG_SW2SPM_INT1_MASK_B:
		val = pwrctrl->reg_sw2spm_int1_mask_b;
		break;
	case PW_REG_SW2SPM_INT2_MASK_B:
		val = pwrctrl->reg_sw2spm_int2_mask_b;
		break;
	case PW_REG_SW2SPM_INT3_MASK_B:
		val = pwrctrl->reg_sw2spm_int3_mask_b;
		break;
	case PW_REG_SC_ADSP2SPM_WAKEUP_MASK_B:
		val = pwrctrl->reg_sc_adsp2spm_wakeup_mask_b;
		break;
	case PW_REG_SC_SSPM2SPM_WAKEUP_MASK_B:
		val = pwrctrl->reg_sc_sspm2spm_wakeup_mask_b;
		break;
	case PW_REG_SC_SCP2SPM_WAKEUP_MASK_B:
		val = pwrctrl->reg_sc_scp2spm_wakeup_mask_b;
		break;
	case PW_REG_CSYSPWRREQ_MASK:
		val = pwrctrl->reg_csyspwrreq_mask;
		break;
	case PW_REG_SPM_SRCCLKENA_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_srcclkena_reserved_mask_b;
		break;
	case PW_REG_SPM_INFRA_REQ_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_infra_req_reserved_mask_b;
		break;
	case PW_REG_SPM_APSRC_REQ_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_apsrc_req_reserved_mask_b;
		break;
	case PW_REG_SPM_VRF18_REQ_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_vrf18_req_reserved_mask_b;
		break;
	case PW_REG_SPM_DDR_EN_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_ddr_en_reserved_mask_b;
		break;
	case PW_REG_MCUPM_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_mcupm_srcclkena_mask_b;
		break;
	case PW_REG_MCUPM_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_mcupm_infra_req_mask_b;
		break;
	case PW_REG_MCUPM_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_mcupm_apsrc_req_mask_b;
		break;
	case PW_REG_MCUPM_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_mcupm_vrf18_req_mask_b;
		break;
	case PW_REG_MCUPM_DDR_EN_MASK_B:
		val = pwrctrl->reg_mcupm_ddr_en_mask_b;
		break;
	case PW_REG_MSDC0_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_msdc0_srcclkena_mask_b;
		break;
	case PW_REG_MSDC0_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_msdc0_infra_req_mask_b;
		break;
	case PW_REG_MSDC0_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_msdc0_apsrc_req_mask_b;
		break;
	case PW_REG_MSDC0_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_msdc0_vrf18_req_mask_b;
		break;
	case PW_REG_MSDC0_DDR_EN_MASK_B:
		val = pwrctrl->reg_msdc0_ddr_en_mask_b;
		break;
	case PW_REG_MSDC1_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_msdc1_srcclkena_mask_b;
		break;
	case PW_REG_MSDC1_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_msdc1_infra_req_mask_b;
		break;
	case PW_REG_MSDC1_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_msdc1_apsrc_req_mask_b;
		break;
	case PW_REG_MSDC1_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_msdc1_vrf18_req_mask_b;
		break;
	case PW_REG_MSDC1_DDR_EN_MASK_B:
		val = pwrctrl->reg_msdc1_ddr_en_mask_b;
		break;

	/* SPM_SRC4_MASK */
	case PW_CCIF_EVENT_MASK_B:
		val = pwrctrl->ccif_event_mask_b;
		break;
	case PW_REG_BAK_PSRI_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_bak_psri_srcclkena_mask_b;
		break;
	case PW_REG_BAK_PSRI_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_bak_psri_infra_req_mask_b;
		break;
	case PW_REG_BAK_PSRI_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_bak_psri_apsrc_req_mask_b;
		break;
	case PW_REG_BAK_PSRI_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_bak_psri_vrf18_req_mask_b;
		break;
	case PW_REG_BAK_PSRI_DDR_EN_MASK_B:
		val = pwrctrl->reg_bak_psri_ddr_en_mask_b;
		break;
	case PW_REG_DRAMC0_MD32_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_dramc0_md32_infra_req_mask_b;
		break;
	case PW_REG_DRAMC0_MD32_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_dramc0_md32_vrf18_req_mask_b;
		break;
	case PW_REG_DRAMC1_MD32_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_dramc1_md32_infra_req_mask_b;
		break;
	case PW_REG_DRAMC1_MD32_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_dramc1_md32_vrf18_req_mask_b;
		break;
	case PW_REG_CONN_SRCCLKENB2PWRAP_MASK_B:
		val = pwrctrl->reg_conn_srcclkenb2pwrap_mask_b;
		break;
	case PW_REG_DRAMC0_MD32_WAKEUP_MASK:
		val = pwrctrl->reg_dramc0_md32_wakeup_mask;
		break;
	case PW_REG_DRAMC1_MD32_WAKEUP_MASK:
		val = pwrctrl->reg_dramc1_md32_wakeup_mask;
		break;

	/* SPM_SRC5_MASK */
	case PW_REG_MCUSYS_MERGE_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_mcusys_merge_apsrc_req_mask_b;
		break;
	case PW_REG_MCUSYS_MERGE_DDR_EN_MASK_B:
		val = pwrctrl->reg_mcusys_merge_ddr_en_mask_b;
		break;
	case PW_REG_MSDC2_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_msdc2_srcclkena_mask_b;
		break;
	case PW_REG_MSDC2_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_msdc2_infra_req_mask_b;
		break;
	case PW_REG_MSDC2_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_msdc2_apsrc_req_mask_b;
		break;
	case PW_REG_MSDC2_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_msdc2_vrf18_req_mask_b;
		break;
	case PW_REG_MSDC2_DDR_EN_MASK_B:
		val = pwrctrl->reg_msdc2_ddr_en_mask_b;
		break;
	case PW_REG_PCIE_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_pcie_srcclkena_mask_b;
		break;
	case PW_REG_PCIE_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_pcie_infra_req_mask_b;
		break;
	case PW_REG_PCIE_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_pcie_apsrc_req_mask_b;
		break;
	case PW_REG_PCIE_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_pcie_vrf18_req_mask_b;
		break;
	case PW_REG_PCIE_DDR_EN_MASK_B:
		val = pwrctrl->reg_pcie_ddr_en_mask_b;
		break;
	/* SPM_WAKEUP_EVENT_MASK */
	case PW_REG_WAKEUP_EVENT_MASK:
		val = pwrctrl->reg_wakeup_event_mask;
		break;

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	case PW_REG_EXT_WAKEUP_EVENT_MASK:
		val = pwrctrl->reg_ext_wakeup_event_mask;
		break;

	default:
		ERROR("%s: %d is undefined!!!\n", __func__, index);
	}

	return val;
}

unsigned int get_spm_last_wakesta(uint64_t target)
{
	unsigned int val = 0;
	struct wake_status *wake_sta = NULL;

	spm_conservation_get_result(&wake_sta);

	if (!wake_sta)
		return 0;

	switch (target) {
	case WAKE_STA_R12:
		val = wake_sta->tr.comm.r12;
		break;
	case WAKE_STA_DEBUG_FLAG:
		val = wake_sta->tr.comm.debug_flag;
		break;
	default:
		break;
	}

	return val;
}

unsigned int get_spm_last_dbgsta(struct dbg_ctrl *dbgctrl,
					uint64_t target)
{

	unsigned int val = 0;

	switch (target) {
	case DBG_CTRL_COUNT:
		val = dbgctrl->count;
		break;
	case DBG_CTRL_DURATION:
		val = dbgctrl->duration;
		break;
	default:
		break;
	}

	return val;
}

uint64_t mt_spm_dbg_dispatcher(uint64_t lp_id, uint64_t act,
					uint64_t arg1, uint64_t arg2,
					void *cookie,
					void *handle,
					uint64_t flags)
{
	uint64_t ret = 0;
	struct constraint_status d_con;
	struct resource_req_status d_req;

	d_con.cond_res = NULL;

	switch (lp_id) {
	case MT_SPM_DBG_SMC_UID_IDLE_PWR_CTRL:
		do {
			struct spm_lp_scen *spmlp;

			mt_spm_idle_generic_get_spm_lp(&spmlp);
			if (!spmlp)
				break;
			if (act & MT_LPM_SMC_ACT_GET)
				ret = load_pwr_ctrl(spmlp->pwrctrl, arg1);
			else if (act & MT_LPM_SMC_ACT_SET)
				save_pwr_ctrl(spmlp->pwrctrl, arg1, arg2);
		} while (0);
		break;
	case MT_SPM_DBG_SMC_UID_IDLE_CNT:
		do {
			struct rc_common_state st;

			st.id = MT_RM_CONSTRAINT_ID_ALL;
			st.act = act;
			st.type = CONSTRAINT_GET_ENTER_CNT;
			st.value = (void *)&d_con;
			d_con.enter_cnt = 0;
			mt_lp_rm_get_status(PLAT_RC_STATUS, &st);
			ret = d_con.enter_cnt;
		} while (0);
		break;
	case MT_SPM_DBG_SMC_UID_SUSPEND_PWR_CTRL:
		do {
			struct spm_lp_scen *spmlp;

			mt_spm_suspend_get_spm_lp(&spmlp);
			if (!spmlp)
				break;
			if (act & MT_LPM_SMC_ACT_GET)
				ret = load_pwr_ctrl(spmlp->pwrctrl, arg1);
			else if (act & MT_LPM_SMC_ACT_SET)
				save_pwr_ctrl(spmlp->pwrctrl, arg1, arg2);
		} while (0);
		break;
	case MT_SPM_DBG_SMC_UID_SUSPEND_DBG_CTRL:
		do {
			struct spm_lp_scen *spmlp;

			mt_spm_suspend_get_spm_lp(&spmlp);
			if (!spmlp)
				break;
			if (act & MT_LPM_SMC_ACT_GET)
				ret = get_spm_last_dbgsta(spmlp->dbgctrl, arg1);
		} while (0);
		break;
	case MT_SPM_DBG_SMC_UID_FS:
		if (act & MT_LPM_SMC_ACT_GET)
			ret = get_spm_last_wakesta(arg1);
		break;
	case MT_SPM_DBG_SMC_UID_RC_SWITCH:
		if (act & MT_LPM_SMC_ACT_GET) {
			struct rc_common_state st;

			st.id = arg1;
			st.act = act;
			st.type = CONSTRAINT_GET_VALID;
			st.value = (void *)&d_con;
			mt_lp_rm_get_status(PLAT_RC_STATUS, &st);
			ret = IS_MT_RM_RC_READY(d_con.is_valid) ? 1 : 0;
		}  else {
			d_con.is_valid = MT_SPM_RC_VALID_SW;
			contraint_update_status((unsigned int)arg1,
					(unsigned int)act,
					CONSTRAINT_UPDATE_VALID,
					(void *)&d_con);
		}
		break;
	case MT_SPM_DBG_SMC_UID_COND_CHECK:
		if (act & MT_LPM_SMC_ACT_GET) {
			struct rc_common_state st;

			st.id = arg1;
			st.act = act;
			st.type = CONSTRAINT_GET_VALID;
			st.value = (void *)&d_con;
			mt_lp_rm_get_status(PLAT_RC_STATUS, &st);
			ret = !!(d_con.is_valid &
				MT_SPM_RC_VALID_COND_CHECK);
		} else {
			d_con.is_valid = MT_SPM_RC_VALID_COND_CHECK;
			contraint_update_status((unsigned int)arg1,
					(unsigned int)act,
					CONSTRAINT_UPDATE_VALID,
					(void *)&d_con);
		}
		break;
	case MT_SPM_DBG_SMC_UID_COND_BLOCK:
		d_con.id = arg1;
		if (act & MT_LPM_SMC_ACT_GET) {
			struct rc_common_state st;

			st.id = arg1;
			st.act = act;
			st.type = CONSTRAINT_COND_BLOCK;
			st.value = (void *)&d_con;
			mt_lp_rm_get_status(PLAT_RC_STATUS, &st);
			ret = d_con.is_cond_block;
		}
		break;
	case MT_SPM_DBG_SMC_UID_BLOCK_LATCH:
		if (!(act & MT_LPM_SMC_ACT_GET)) {
			d_con.is_valid = MT_SPM_RC_VALID_COND_LATCH;
			contraint_update_status((unsigned int)arg1,
					(unsigned int)act,
					CONSTRAINT_UPDATE_VALID,
					(void *)&d_con);
		}
		break;
	case MT_SPM_DBG_SMC_UID_BLOCK_DETAIL:
		if (act & MT_LPM_SMC_ACT_GET) {
			struct rc_common_state st;

			d_con.cond_res = NULL;
			st.id = arg1;
			st.act = act;
			st.type = CONSTRAINT_GET_COND_BLOCK_DETAIL;
			st.value = (void *)&d_con;
			mt_lp_rm_get_status(PLAT_RC_STATUS, &st);
			if (d_con.cond_res &&
			   (arg2 < PLAT_SPM_COND_MAX))
				ret = d_con.cond_res->table_cg[arg2];
			else
				ret = 0;
		}
		break;
	case MT_SPM_DBG_SMC_UID_RC_CNT:
		if (act & MT_LPM_SMC_ACT_GET) {
			struct rc_common_state st;

			st.id = arg1;
			st.act = act;
			st.type = CONSTRAINT_GET_ENTER_CNT;
			st.value = (void *)&d_con;
			mt_lp_rm_get_status(PLAT_RC_STATUS, &st);
			ret = d_con.enter_cnt;
		}
		break;
	case MT_SPM_DBG_SMC_UID_RES_REQ:
		d_req.id = arg1;
		d_req.val = arg2;
		if (act & MT_LPM_SMC_ACT_SET)
			mt_lp_rq_update_status(PLAT_RQ_USER_REQ, &d_req);
		else if (act & MT_LPM_SMC_ACT_CLR)
			mt_lp_rq_update_status(PLAT_RQ_USER_REL, &d_req);
		break;
	case MT_SPM_DBG_SMC_UID_RES_USER_NUM:
		d_req.id = arg1;
		if (act & MT_LPM_SMC_ACT_GET) {
			mt_lp_rq_get_status(PLAT_RQ_USER_NUM, &d_req);
			ret = d_req.val;
		}
		break;
	case MT_SPM_DBG_SMC_UID_RES_USER_VALID:
		d_req.id = arg1;
		d_req.val = arg2;
		if (act & MT_LPM_SMC_ACT_GET) {
			mt_lp_rq_get_status(PLAT_RQ_USER_VALID, &d_req);
			ret = d_req.val;
		} else if (act & MT_LPM_SMC_ACT_SET)
			mt_lp_rq_update_status(PLAT_RQ_USER_VALID, &d_req);
		break;
	case MT_SPM_DBG_SMC_UID_RES_USER_NAME:
		d_req.id = arg1;
		if (act & MT_LPM_SMC_ACT_GET) {
			mt_lp_rq_get_status(PLAT_RQ_PER_USER_NAME, &d_req);
			ret = d_req.val;
		}
		break;
	case MT_SPM_DBG_SMC_UID_RES_USAGE:
		d_req.id = arg1;
		if (act & MT_LPM_SMC_ACT_GET) {
			mt_lp_rq_get_status(PLAT_RQ_REQ_USAGE, &d_req);
			ret = d_req.val;
		}
		break;
	case MT_SPM_DBG_SMC_UID_RES_NUM:
		d_req.id = arg1;
		if (act & MT_LPM_SMC_ACT_GET) {
			mt_lp_rq_get_status(PLAT_RQ_REQ_NUM, &d_req);
			ret = d_req.val;
		}
		break;
	case MT_SPM_DBG_SMC_UID_DOE_RESOURCE_CTRL:
		do {
			struct spm_lp_scen *spmlp;

			mt_spm_idle_generic_get_spm_lp(&spmlp);
			if (!spmlp)
				break;
			save_pwr_ctrl(spmlp->pwrctrl,
				      mt8192_resource_ctrl[arg1], arg2);
			mt_spm_suspend_get_spm_lp(&spmlp);
			if (!spmlp)
				break;
			save_pwr_ctrl(spmlp->pwrctrl,
				      mt8192_resource_ctrl[arg1], arg2);
		} while (0);
		break;
	case MT_SPM_DBG_SMC_UID_DOE_RC:
		{
			d_con.is_valid = MT_SPM_RC_VALID_SW;
			contraint_update_status((unsigned int)arg1,
					(unsigned int)act,
					CONSTRAINT_UPDATE_VALID,
					(void *)&d_con);
		}
		break;
	case MT_SPM_DBG_SMC_UID_RC_COND_CTRL:
		contraint_update_status((unsigned int)arg1,
					(unsigned int)act,
					CONSTRAINT_UPDATE_COND_CHECK,
					(void *)&arg2);
		break;
	case MT_SPM_DBG_SMC_UID_RC_RES_CTRL:
		d_con.is_valid = MT_SPM_RC_VALID_RESIDNECY;

		if (act & MT_LPM_SMC_ACT_GET) {
			struct rc_common_state st;

			st.id = arg1;
			st.act = act;
			st.type = CONSTRAINT_GET_VALID;
			st.value = (void *)&d_con;
			mt_lp_rm_get_status(PLAT_RC_STATUS, &st);
			ret = !!(d_con.is_valid & MT_SPM_RC_VALID_RESIDNECY);
		} else {
			contraint_update_status((unsigned int)arg1,
						(unsigned int)act,
						CONSTRAINT_UPDATE_VALID,
						(void *)&d_con);
			/* if clear constraints residency checking
			 * and also clear residency value
			 */
			if (act & MT_LPM_SMC_ACT_CLR) {
				contraint_update_status((unsigned int)arg1,
						(unsigned int)act,
						CONSTRAINT_RESIDNECY,
						(void *)&d_con);
			}
		}
		break;
	case MT_SPM_DBG_SMC_UID_RC_RES_INFO:
		if (act & MT_LPM_SMC_ACT_GET) {
			struct rc_common_state st;

			st.id = arg1;
			st.act = act;
			st.type = CONSTRAINT_GET_RESIDNECY;
			st.value = (void *)&d_con;
			mt_lp_rm_get_status(PLAT_RC_STATUS, &st);
			ret = (uint64_t)d_con.residency;
		}
		break;
	case MT_SPM_DBG_SMC_UID_RC_BBLPM:
		contraint_do_valid((unsigned int)arg1, (unsigned int)act,
				    MT_SPM_RC_VALID_XSOC_BBLPM, &d_con,
				    (unsigned int *)&ret);
		break;
	case MT_SPM_DBG_SMC_UID_RC_TRACE:
		contraint_do_valid((unsigned int)arg1, (unsigned int)act,
				    MT_SPM_RC_VALID_TRACE_EVENT, &d_con,
				    (unsigned int *)&ret);
		break;
	default:
		break;
	}
	return ret;
}

int mt_spm_dbg_dispatcher_init(void)
{
	mt_lpm_dispatcher_registry(mt_lpm_smc_user_spm_dbg,
				   mt_spm_dbg_dispatcher);
	return 0;
}

