#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <mt_spm_pmic_wrap.h>
#include <plat_pm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define VCORE_CT_ENABLE (1U << 5)

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= R12_REG_CPU_WAKEUP,

	/* default VCORE DVFS is disabled */
	.pcm_flags = (SPM_FLAG_RUN_COMMON_SCENARIO |
			SPM_FLAG_DISABLE_VCORE_DVS | SPM_FLAG_DISABLE_VCORE_DFS),

	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
	.reg_wfi_op = 0,
	.reg_wfi_type = 0,
	.reg_mp0_cputop_idle_mask = 0,
	.reg_mp1_cputop_idle_mask = 0,
	.reg_mcusys_idle_mask = 0,
	.reg_md_apsrc_1_sel = 0,
	.reg_md_apsrc_0_sel = 0,
	.reg_conn_apsrc_sel = 0,

	/* SPM_SRC_REQ */
	.reg_spm_apsrc_req = 0,
	.reg_spm_f26m_req = 0,
	.reg_spm_infra_req = 0,
	.reg_spm_vrf18_req = 0,
	.reg_spm_ddr_en_req = 1,/* FIXME: default disable HW Auto S1 */
	.reg_spm_dvfs_req = 0,
	.reg_spm_sw_mailbox_req = 0,
	.reg_spm_sspm_mailbox_req = 0,
	.reg_spm_adsp_mailbox_req = 0,
	.reg_spm_scp_mailbox_req = 0,

	/* SPM_SRC6_MASK */
	.reg_dpmaif_srcclkena_mask_b = 1,
	.reg_dpmaif_infra_req_mask_b = 1,
	.reg_dpmaif_apsrc_req_mask_b = 1,
	.reg_dpmaif_vrf18_req_mask_b = 1,
	.reg_dpmaif_ddr_en_mask_b    = 1,

	/* SPM_SRC_MASK */
	.reg_md_srcclkena_0_mask_b = 1,
	.reg_md_srcclkena2infra_req_0_mask_b = 0,
	.reg_md_apsrc2infra_req_0_mask_b = 1,
	.reg_md_apsrc_req_0_mask_b = 1,
	.reg_md_vrf18_req_0_mask_b = 1,
	.reg_md_ddr_en_0_mask_b = 1,
	.reg_md_srcclkena_1_mask_b = 0,
	.reg_md_srcclkena2infra_req_1_mask_b = 0,
	.reg_md_apsrc2infra_req_1_mask_b = 0,
	.reg_md_apsrc_req_1_mask_b = 0,
	.reg_md_vrf18_req_1_mask_b = 0,
	.reg_md_ddr_en_1_mask_b = 0,
	.reg_conn_srcclkena_mask_b = 1,
	.reg_conn_srcclkenb_mask_b = 0,
	.reg_conn_infra_req_mask_b = 1,
	.reg_conn_apsrc_req_mask_b = 1,
	.reg_conn_vrf18_req_mask_b = 1,
	.reg_conn_ddr_en_mask_b = 1,
	.reg_conn_vfe28_mask_b = 0,
	.reg_srcclkeni0_srcclkena_mask_b = 1,
	.reg_srcclkeni0_infra_req_mask_b = 1,
	.reg_srcclkeni1_srcclkena_mask_b = 0,
	.reg_srcclkeni1_infra_req_mask_b = 0,
	.reg_srcclkeni2_srcclkena_mask_b = 0,
	.reg_srcclkeni2_infra_req_mask_b = 0,
	.reg_infrasys_apsrc_req_mask_b = 0,
	.reg_infrasys_ddr_en_mask_b = 1,
	.reg_md32_srcclkena_mask_b = 1,
	.reg_md32_infra_req_mask_b = 1,
	.reg_md32_apsrc_req_mask_b = 1,
	.reg_md32_vrf18_req_mask_b = 1,
	.reg_md32_ddr_en_mask_b = 1,

	/* SPM_SRC2_MASK */
	.reg_scp_srcclkena_mask_b = 1,
	.reg_scp_infra_req_mask_b = 1,
	.reg_scp_apsrc_req_mask_b = 1,
	.reg_scp_vrf18_req_mask_b = 1,
	.reg_scp_ddr_en_mask_b = 1,
	.reg_audio_dsp_srcclkena_mask_b = 1,
	.reg_audio_dsp_infra_req_mask_b = 1,
	.reg_audio_dsp_apsrc_req_mask_b = 1,
	.reg_audio_dsp_vrf18_req_mask_b = 1,
	.reg_audio_dsp_ddr_en_mask_b = 1,
	.reg_ufs_srcclkena_mask_b = 1,
	.reg_ufs_infra_req_mask_b = 1,
	.reg_ufs_apsrc_req_mask_b = 1,
	.reg_ufs_vrf18_req_mask_b = 1,
	.reg_ufs_ddr_en_mask_b = 1,
	.reg_disp0_apsrc_req_mask_b = 1,
	.reg_disp0_ddr_en_mask_b = 1,
	.reg_disp1_apsrc_req_mask_b = 1,
	.reg_disp1_ddr_en_mask_b = 1,
	.reg_gce_infra_req_mask_b = 1,
	.reg_gce_apsrc_req_mask_b = 1,
	.reg_gce_vrf18_req_mask_b = 1,
	.reg_gce_ddr_en_mask_b = 1,
	.reg_apu_srcclkena_mask_b = 1,
	.reg_apu_infra_req_mask_b = 1,
	.reg_apu_apsrc_req_mask_b = 1,
	.reg_apu_vrf18_req_mask_b = 1,
	.reg_apu_ddr_en_mask_b = 1,
	.reg_cg_check_srcclkena_mask_b = 0,/* FIXME: go SW CG check first */
	.reg_cg_check_apsrc_req_mask_b = 0,/* FIXME: go SW CG check first */
	.reg_cg_check_vrf18_req_mask_b = 0,/* FIXME: go SW CG check first */
	.reg_cg_check_ddr_en_mask_b = 0,/* FIXME: go SW CG check first */

	/* SPM_SRC3_MASK */
	.reg_dvfsrc_event_trigger_mask_b = 1,
	.reg_sw2spm_int0_mask_b = 0,
	.reg_sw2spm_int1_mask_b = 0,
	.reg_sw2spm_int2_mask_b = 0,
	.reg_sw2spm_int3_mask_b = 0,
	.reg_sc_adsp2spm_wakeup_mask_b = 0,
	.reg_sc_sspm2spm_wakeup_mask_b = 0,
	.reg_sc_scp2spm_wakeup_mask_b = 0,
	.reg_csyspwrreq_mask = 1,
	.reg_spm_srcclkena_reserved_mask_b = 0,
	.reg_spm_infra_req_reserved_mask_b = 0,
	.reg_spm_apsrc_req_reserved_mask_b = 0,
	.reg_spm_vrf18_req_reserved_mask_b = 0,
	.reg_spm_ddr_en_reserved_mask_b = 0,
	.reg_mcupm_srcclkena_mask_b = 1,
	.reg_mcupm_infra_req_mask_b = 1,
	.reg_mcupm_apsrc_req_mask_b = 1,
	.reg_mcupm_vrf18_req_mask_b = 1,
	.reg_mcupm_ddr_en_mask_b = 1,
	.reg_msdc0_srcclkena_mask_b = 1,
	.reg_msdc0_infra_req_mask_b = 1,
	.reg_msdc0_apsrc_req_mask_b = 1,
	.reg_msdc0_vrf18_req_mask_b = 1,
	.reg_msdc0_ddr_en_mask_b = 1,
	.reg_msdc1_srcclkena_mask_b = 1,
	.reg_msdc1_infra_req_mask_b = 1,
	.reg_msdc1_apsrc_req_mask_b = 1,
	.reg_msdc1_vrf18_req_mask_b = 1,
	.reg_msdc1_ddr_en_mask_b = 1,

	/* SPM_SRC4_MASK */
	.ccif_event_mask_b = 0xFFF,
	.reg_bak_psri_srcclkena_mask_b = 0,
	.reg_bak_psri_infra_req_mask_b = 0,
	.reg_bak_psri_apsrc_req_mask_b = 0,
	.reg_bak_psri_vrf18_req_mask_b = 0,
	.reg_bak_psri_ddr_en_mask_b = 0,
	.reg_dramc0_md32_infra_req_mask_b = 1,
	.reg_dramc0_md32_vrf18_req_mask_b = 0,
	.reg_dramc1_md32_infra_req_mask_b = 1,
	.reg_dramc1_md32_vrf18_req_mask_b = 0,
	.reg_conn_srcclkenb2pwrap_mask_b = 0,
	.reg_dramc0_md32_wakeup_mask = 1,
	.reg_dramc1_md32_wakeup_mask = 1,

	/* SPM_SRC5_MASK */
	.reg_mcusys_merge_apsrc_req_mask_b = 0x11,
	.reg_mcusys_merge_ddr_en_mask_b = 0x11,
	.reg_msdc2_srcclkena_mask_b = 1,
	.reg_msdc2_infra_req_mask_b = 1,
	.reg_msdc2_apsrc_req_mask_b = 1,
	.reg_msdc2_vrf18_req_mask_b = 1,
	.reg_msdc2_ddr_en_mask_b = 1,
	.reg_pcie_srcclkena_mask_b = 1,
	.reg_pcie_infra_req_mask_b = 1,
	.reg_pcie_apsrc_req_mask_b = 1,
	.reg_pcie_vrf18_req_mask_b = 1,
	.reg_pcie_ddr_en_mask_b = 1,

	/* SPM_WAKEUP_EVENT_MASK */
	.reg_wakeup_event_mask = 0xEFFFFFFF,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,

	/* Auto-gen End */
};

struct spm_lp_scen __spm_vcorefs = {
	.pwrctrl	= &vcorefs_ctrl,
};

#define DVFSRC_1600_FLOOR

static struct reg_config dvfsrc_init_configs[][128] = {
	{
		{ -1, 0 },
	},
};

static int spm_dvfs_init_done;
static int dvfs_enable_done;
static int vcore_opp_0_uv = 725000;
static int vcore_opp_1_uv = 650000;
static int vcore_opp_2_uv = 600000;
static int vcore_opp_3_uv = 575000;


static void dvfsrc_init(void)
{
	struct reg_config *config;
	int idx = 0;

	if (dvfs_enable_done)
		return;

	config = dvfsrc_init_configs[0];

	while (config[idx].offset != -1) {
		mmio_write_32(config[idx].offset, config[idx].val);
		idx++;
	}

	dvfs_enable_done = 1;
}

static void spm_vcorefs_pwarp_cmd(uint64_t cmd, uint64_t val)
{
	if (cmd < NR_IDX_ALL)
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, cmd, val);
	else
		INFO("cmd out of range!\n");
}

static void spm_vcorefs_vcore_setting(uint64_t flag)
{


	switch (flag) {
	case 1: /*HV*/
		vcore_opp_0_uv = 725000;
		vcore_opp_1_uv = 650000;
		vcore_opp_2_uv = 600000;
		vcore_opp_3_uv = 575000;
		break;
	case 2: /*LV*/
		vcore_opp_0_uv = 725000;
		vcore_opp_1_uv = 650000;
		vcore_opp_2_uv = 600000;
		vcore_opp_3_uv = 575000;
		break;
	case 3: /*AGING*/
		vcore_opp_0_uv = 725000 - 12500;
		vcore_opp_1_uv = 650000 - 12500;
		vcore_opp_2_uv = 600000 - 12500;
		vcore_opp_3_uv = 575000 - 12500;
		break;
	default:
		break;
	}

	spm_vcorefs_pwarp_cmd(3, __vcore_uv_to_pmic(vcore_opp_3_uv));
	spm_vcorefs_pwarp_cmd(2, __vcore_uv_to_pmic(vcore_opp_2_uv));
	spm_vcorefs_pwarp_cmd(1, __vcore_uv_to_pmic(vcore_opp_1_uv));
	spm_vcorefs_pwarp_cmd(0, __vcore_uv_to_pmic(vcore_opp_0_uv));

}

static int spm_vcorefs_get_vcore(int gear)
{
	switch (gear) {
	case 2:
		return vcore_opp_0_uv;
	case 1:
		return vcore_opp_1_uv;
	case 0:
	default:
		return vcore_opp_2_uv;
	}
}

void spm_request_dvfs_opp(uint64_t id, uint64_t reg)
{
}

void spm_dvfsfw_init(uint64_t boot_up_opp, uint64_t dram_issue)
{
	if (spm_dvfs_init_done)
		return;

	mmio_write_32(SPM_DVFS_MISC, (mmio_read_32(SPM_DVFS_MISC) &
		~(SPM_DVFS_FORCE_ENABLE_LSB)) | (SPM_DVFSRC_ENABLE_LSB));

	mmio_write_32(SPM_DVFS_LEVEL, 0x00000001);
	mmio_write_32(SPM_DVS_DFS_LEVEL, 0x00010001);

	spm_dvfs_init_done = 1;
}

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl, const struct pwr_ctrl *src_pwr_ctrl)
{
#if 1
	uint32_t dvfs_mask = SPM_FLAG_DISABLE_VCORE_DVS | SPM_FLAG_DISABLE_VCORE_DFS | SPM_FLAG_ENABLE_VOLTAGE_BIN;

	dest_pwr_ctrl->pcm_flags = (dest_pwr_ctrl->pcm_flags & (~dvfs_mask)) |
					(src_pwr_ctrl->pcm_flags & dvfs_mask);

	if (dest_pwr_ctrl->pcm_flags_cust)
		dest_pwr_ctrl->pcm_flags_cust = (dest_pwr_ctrl->pcm_flags_cust & (~dvfs_mask)) |
						(src_pwr_ctrl->pcm_flags & dvfs_mask);
#endif
}

void dvfsrc_md_ddr_turbo(int is_turbo)
{
	if (!mmio_read_32(DVFSRC_RSRV_5))
		return;

	if (is_turbo)
		mmio_write_32(DVFSRC_MD_TURBO, 0x00000000);
	else
		mmio_write_32(DVFSRC_MD_TURBO, 0x1FFF0000);
}

void spm_go_to_vcorefs(uint64_t spm_flags)
{
	set_pwrctrl_pcm_flags(__spm_vcorefs.pwrctrl, spm_flags);
}

unsigned int spm_vcorefs_type_wrapp(unsigned int fwtype)
{
	return 0;
}

uint64_t spm_vcorefs_args(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint64_t ret = 0;
	uint64_t cmd = x1;

	switch (cmd) {
	case VCOREFS_SMC_CMD_0:
		spm_dvfsfw_init(x2, x3);
		break;
	case VCOREFS_SMC_CMD_1:
		if (mmio_read_32(DVFSRC_RSRV_4) & VCORE_CT_ENABLE)
			x2 |= SPM_FLAG_ENABLE_VOLTAGE_BIN;

		spm_go_to_vcorefs(x2);
		break;
	case VCOREFS_SMC_CMD_2:
		spm_request_dvfs_opp(x2, x3);
		break;
	case VCOREFS_SMC_CMD_3:
		spm_vcorefs_pwarp_cmd(x2, x3);
		break;
	case VCOREFS_SMC_CMD_6:
		ret = spm_vcorefs_type_wrapp(0);
		break;
	case VCOREFS_SMC_CMD_4:
	case VCOREFS_SMC_CMD_7:
	default:
		break;
	}
	return ret;
}


uint64_t spm_vcorefs_v2_args(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t *x4)
{
	uint64_t ret = 0;
	uint64_t cmd = x1;
	uint64_t spm_flags;

	switch (cmd) {
	case VCOREFS_SMC_CMD_0:  /* vcore_dvfs init + kick */
		spm_dvfsfw_init(0, 0);
		spm_vcorefs_vcore_setting(x3 & 0xF);
		spm_flags = x2 & 0xF;
		spm_flags = SPM_FLAG_RUN_COMMON_SCENARIO;
		if (x2 & 0x1)
			spm_flags |= SPM_FLAG_DISABLE_VCORE_DVS;

		if (x2 & 0x2)
			spm_flags |= SPM_FLAG_DISABLE_VCORE_DFS;

		spm_go_to_vcorefs(spm_flags);
		dvfsrc_init();
		*x4 = 0;
		break;
	case VCOREFS_SMC_CMD_1:
		break;
	case VCOREFS_SMC_CMD_2:  /* get dram type */
		*x4 = 0;
		break;
	case VCOREFS_SMC_CMD_3:
		spm_vcorefs_pwarp_cmd(x2, x3);
		break;
	case VCOREFS_SMC_CMD_4:
		*x4 = spm_vcorefs_get_vcore(x2);
		break;
	default:
		break;
	}

	return ret;
}

