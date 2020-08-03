#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mt_lp_rqm.h>
#include <mt_spm.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mt_spm_conservation.h>
#include <mt_spm.h>
#include <mt_spm_trace.h>

struct wake_status spm_wakesta; /* record last wakesta */
static wake_reason_t spm_wake_reason = WR_NONE;
static struct resource_req_status generic_spm_resource_req = {
	.id = MT_LP_RQ_ID_ALL_USAGE,
	.val = 0,
};

#ifndef ATF_PLAT_SPM_TRACE_UNSUPPORT
#define SPM_CONSERVATION_BOUND		(MT_SPM_TRACE_LP_RINGBUF_MAX - 1)

static unsigned int spm_conservation_trace_lp_idx;
static const unsigned int spm_conservation_trace_comm_sz =
			sizeof(struct wake_status_trace_comm);
static const unsigned int spm_conservation_trace_sz =
			sizeof(struct wake_status_trace);

static void spm_conservation_trace_lp(struct wake_status_trace *trace)
{
	unsigned int type[MT_SPM_TRACE_LP_RINGBUF_MAX] = {
				MT_SPM_TRACE_LP_RINGBUF_0,
				MT_SPM_TRACE_LP_RINGBUF_1,
				MT_SPM_TRACE_LP_RINGBUF_2,
				MT_SPM_TRACE_LP_RINGBUF_3,
				MT_SPM_TRACE_LP_RINGBUF_4,
			};

	if (!trace)
		return;

	MT_SPM_SYSRAM_LP_W_U32(MT_SPM_TRACE_LP_RINGBUF_IDX,
				(unsigned int)spm_conservation_trace_lp_idx);

	MT_SPM_SYSRAM_LP_W(type[spm_conservation_trace_lp_idx],
				&trace->comm, spm_conservation_trace_comm_sz);

	if (spm_conservation_trace_lp_idx < SPM_CONSERVATION_BOUND)
		spm_conservation_trace_lp_idx += 1;
	else
		spm_conservation_trace_lp_idx = 0;
}

static void spm_conservation_trace_suspend(struct wake_status_trace *trace)
{
	if (!trace)
		return;

	MT_SPM_SYSRAM_SUSPEND_W(MT_SPM_TRACE_SUSPEND_WAKE_SRC,
				trace, spm_conservation_trace_sz);
}

#endif

static int go_to_spm_before_wfi(int state_id, unsigned int ext_opand,
				struct spm_lp_scen *spm_lp,
				unsigned int resource_req)
{
	int ret = 0;
	struct pwr_ctrl *pwrctrl;
	uint32_t cpu = plat_my_core_pos();

	pwrctrl = spm_lp->pwrctrl;

	__spm_set_cpu_status(cpu);
	__spm_set_power_control(pwrctrl);
	__spm_set_wakeup_event(pwrctrl);
	__spm_sync_vcore_dvfs_power_control(pwrctrl, __spm_vcorefs.pwrctrl);

	__spm_set_pcm_flags(pwrctrl);

	__spm_src_req_update(pwrctrl, resource_req);

	if (ext_opand & MT_SPM_EX_OP_CLR_26M_RECORD)
		__spm_clean_before_wfi();

	/* system watchdog is stopped at kernel stage */
	if (ext_opand & MT_SPM_EX_OP_SET_WDT)
		__spm_set_pcm_wdt(0);

	if (ext_opand & MT_SPM_EX_OP_SRCLKEN_RC_BBLPM)
		__spm_xo_soc_bblpm(1);

	if (ext_opand & MT_SPM_EX_OP_HW_S1_DETECT)
		spm_hw_s1_state_monitor_resume();

	__spm_send_cpu_wakeup_event();

	if (IS_PLAT_SUSPEND_ID(state_id))
		__spm_disable_pcm_timer();

	INFO("cpu%d: wakesrc = 0x%x, settle = 0x%x, sec = %u\n",
	     cpu, pwrctrl->wake_src, mmio_read_32(SPM_CLK_SETTLE),
	     mmio_read_32(PCM_TIMER_VAL) / 32768);
	INFO("sw_flag = 0x%x 0x%x, req = 0x%x, pwr = 0x%x 0x%x\n",
	     pwrctrl->pcm_flags, pwrctrl->pcm_flags1,
	     mmio_read_32(SPM_SRC_REQ), mmio_read_32(PWR_STATUS),
	     mmio_read_32(PWR_STATUS_2ND));

	return ret;
}

static void go_to_spm_after_wfi(int state_id, unsigned int ext_opand,
				     struct spm_lp_scen *spm_lp,
				     struct wake_status **status)
{
	unsigned int ext_status = 0;

	/* system watchdog will be resumed at kernel stage */
	if (ext_opand & MT_SPM_EX_OP_SET_WDT)
		__spm_set_pcm_wdt(0);

	if (ext_opand & MT_SPM_EX_OP_SRCLKEN_RC_BBLPM)
		__spm_xo_soc_bblpm(0);

	if (ext_opand & MT_SPM_EX_OP_HW_S1_DETECT)
		spm_hw_s1_state_monitor_pause(&ext_status);

	__spm_ext_int_wakeup_req_clr();

	__spm_get_wakeup_status(&spm_wakesta, ext_status);

	if (status)
		*status = &spm_wakesta;

#ifndef ATF_PLAT_SPM_TRACE_UNSUPPORT
	if (ext_opand & MT_SPM_EX_OP_TRACE_LP)
		spm_conservation_trace_lp(&spm_wakesta.tr);
	else if (ext_opand & MT_SPM_EX_OP_TRACE_SUSPEND)
		spm_conservation_trace_suspend(&spm_wakesta.tr);
#endif
	__spm_clean_after_wakeup();
	spm_wake_reason = __spm_output_wake_reason(state_id, &spm_wakesta);
}

int spm_conservation(int state_id, unsigned int ext_opand,
			  struct spm_lp_scen *spm_lp, unsigned int resource_req)
{
	unsigned int rc_state = resource_req;

	if (!spm_lp)
		return -1;

	spm_lock_get();

	if (!(ext_opand & MT_SPM_EX_OP_NON_GENERIC_RESOURCE_REQ)) {
		/* ATF resource request */
		mt_lp_rq_get_status(PLAT_RQ_REQ_USAGE,
				    &generic_spm_resource_req);
		rc_state |= generic_spm_resource_req.val;
	}
	go_to_spm_before_wfi(state_id, ext_opand, spm_lp, rc_state);
	spm_lock_release();

	return 0;
}

void spm_conservation_finish(int state_id, unsigned int ext_opand,
				   struct spm_lp_scen *spm_lp,
				   struct wake_status **status)
{
	spm_lock_get();
	go_to_spm_after_wfi(state_id, ext_opand, spm_lp, status);
	spm_lock_release();
}

int spm_conservation_get_result(struct wake_status **res)
{
	if (!res)
		return -1;
	*res = &spm_wakesta;
	return 0;
}


#define GPIO_BANK	(GPIO_BASE + 0x6F0)
#define TRAP_UFS_FIRST        (1U << 11) /* bit 11, 0: UFS, 1: eMMC */

void spm_conservation_pwrctrl_init(struct pwr_ctrl *pwrctrl)
{
	if (!pwrctrl)
		return;

	/* for ufs, emmc storage type workaround */
	if ((mmio_read_32(GPIO_BANK) & TRAP_UFS_FIRST) != 0) {
		/* eMMC, mask UFS req */
		pwrctrl->reg_ufs_srcclkena_mask_b = 0;
		pwrctrl->reg_ufs_infra_req_mask_b = 0;
		pwrctrl->reg_ufs_apsrc_req_mask_b = 0;
		pwrctrl->reg_ufs_vrf18_req_mask_b = 0;
		pwrctrl->reg_ufs_ddr_en_mask_b = 0;
	}
}

