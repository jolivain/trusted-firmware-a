/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <arch.h>
#include <lib/bakery_lock.h>
#include <drivers/console.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mt_spm.h>
#include <plat_pm.h>
#include <platform_def.h>
#include <plat_private.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mtk_plat_common.h>
#include <assert.h>
#include <plat_mtk_lpm.h>
#include <mt_lp_rm.h>
#include <mt_lp_rqm.h>
#include <mt_spm_rc_internal.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_conservation.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_pmic_wrap.h>
#include <mt_spm_cond.h>
#include <mt_spm_suspend.h>
#include <mt_spm_idle.h>
#include <mt_spm_constraint.h>
#include <mt_lpm_smc.h>
#include <sleep_def.h>

/* CLK_SCP_CFG_0 */
#define CLK_SCP_CFG_0		(TOPCKGEN_BASE + 0x200)
#define SPM_CK_CONTROL_EN	(0x3FF)

/* CLK_SCP_CFG_1 */
#define CLK_SCP_CFG_1		(TOPCKGEN_BASE + 0x210)
#define CLK_SCP_CFG_1_MASK	(0x100C)
#define CLK_SCP_CFG_1_SPM	(0x3)


#ifdef MT_SPM_USING_BAKERY_LOCK
DEFINE_BAKERY_LOCK(spm_lock);
#define plat_spm_lock_init()\
	bakery_lock_init(&spm_lock)
#else
spinlock_t spm_lock;
#define plat_spm_lock_init()
#endif

void spm_set_sysclk_settle(void)
{
	uint32_t settle;

	mmio_write_32(SPM_CLK_SETTLE, SPM_SYSCLK_SETTLE);
	settle = mmio_read_32(SPM_CLK_SETTLE);

	INFO("md_settle = %u, settle = %u\n", SPM_SYSCLK_SETTLE, settle);
}

static int spm_ap_mdsrc_ack(void)
{
	int ack, md_state;

	/* Check ap_mdsrc_ack = 1'b1, for md internal resource on ack */
	ack = !!(mmio_read_32(AP_MDSRC_REQ) & AP_MDSMSRC_ACK_LSB);

	if (!ack) {
		/* Check md_apsrc_req = 1'b0, for md state 0:sleep, 1:wakeup */
		md_state = !!(mmio_read_32(PCM_REG13_DATA)
			      & R13_MD_APSRC_REQ_0);

		ERROR("[SPM] error: md_sleep = %d\n", md_state);
		ERROR("%s can not get AP_MDSRC_ACK\n", __func__);
		return -1;
	}
	return 0;
}

static void spm_ap_mdsrc_req(int set)
{
	spm_lock_get();

	if (set)
		mmio_write_32(AP_MDSRC_REQ, (mmio_read_32(AP_MDSRC_REQ) |
			      AP_MDSMSRC_REQ_LSB));
	else
		mmio_write_32(AP_MDSRC_REQ, mmio_read_32(AP_MDSRC_REQ) &
			      ~AP_MDSMSRC_REQ_LSB);

	spm_lock_release();
}

struct mt_lp_res_req rq_xo_fpm = {
	.res_id = MT_LP_RQ_XO_FPM,
	.res_rq = MT_SPM_XO_FPM,
	.res_usage = 0,
};

struct mt_lp_res_req rq_26m = {
	.res_id = MT_LP_RQ_26M,
	.res_rq = MT_SPM_26M,
	.res_usage = 0,
};

struct mt_lp_res_req rq_infra = {
	.res_id = MT_LP_RQ_INFRA,
	.res_rq = MT_SPM_INFRA,
	.res_usage = 0,
};

struct mt_lp_res_req rq_syspll = {
	.res_id = MT_LP_RQ_SYSPLL,
	.res_rq = MT_SPM_SYSPLL,
	.res_usage = 0,
};

struct mt_lp_res_req rq_dram_s0 = {
	.res_id = MT_LP_RQ_DRAM,
	.res_rq = MT_SPM_DRAM_S0,
	.res_usage = 0,
};

struct mt_lp_res_req rq_dram_s1 = {
	.res_id = MT_LP_RQ_DRAM,
	.res_rq = MT_SPM_DRAM_S1,
	.res_usage = 0,
};

struct mt_lp_res_req *spm_resources[] = {
	&rq_xo_fpm,
	&rq_26m,
	&rq_infra,
	&rq_syspll,
	&rq_dram_s0,
	&rq_dram_s1,
	NULL,
};

struct mt_resource_req_manager plat_mt8192_rq = {
	.res = spm_resources,
};

struct mt_resource_constraint plat_constraint_bus26m = {
	.is_valid = spm_is_valid_rc_bus26m,
	.update = spm_update_rc_bus26m,
	.allow = spm_allow_rc_bus26m,
	.run = spm_run_rc_bus26m,
	.reset = spm_reset_rc_bus26m,
	.get_status = spm_get_status_rc_bus26m,
};

struct mt_resource_constraint plat_constraint_syspll = {
	.is_valid = spm_is_valid_rc_syspll,
	.update = spm_update_rc_syspll,
	.allow = spm_allow_rc_syspll,
	.run = spm_run_rc_syspll,
	.reset = spm_reset_rc_syspll,
	.get_status = spm_get_status_rc_syspll,
};

struct mt_resource_constraint plat_constraint_dram = {
	.is_valid = spm_is_valid_rc_dram,
	.update = spm_update_rc_dram,
	.allow = spm_allow_rc_dram,
	.run = spm_run_rc_dram,
	.reset = spm_reset_rc_dram,
	.get_status = spm_get_status_rc_dram,
};

/* Maybe remove when the spm won't cpu power control aymore */
struct mt_resource_constraint plat_constraint_cpu = {
	.is_valid = spm_is_valid_rc_cpu_buck_ldo,
	.update = spm_update_rc_cpu_buck_ldo,
	.allow = spm_allow_rc_cpu_buck_ldo,
	.run = spm_run_rc_cpu_buck_ldo,
	.reset = spm_reset_rc_cpu_buck_ldo,
	.get_status = spm_get_status_rc_cpu_buck_ldo,
};
struct mt_resource_constraint *plat_constraints[] = {
	&plat_constraint_bus26m,
	&plat_constraint_syspll,
	&plat_constraint_dram,
	&plat_constraint_cpu,
	NULL,
};

int mt_spm_hwctrl(int type, int set, void *priv)
{
	int ret = 0;

	if (type == PLAT_AP_MDSRC_REQ)
		spm_ap_mdsrc_req(set);
	else if (type == PLAT_AP_MDSRC_ACK)
		ret = spm_ap_mdsrc_ack();
	else if (type == PLAT_AP_MDSRC_SETTLE)
		*(int *)priv = AP_MDSRC_REQ_MD_26M_SETTLE;

	return ret;
}

struct mt_resource_manager plat_mt8192_rm = {
	.update = mt_spm_cond_update,
	.hwctrl = mt_spm_hwctrl,
	.consts = plat_constraints,
};

static void spm_notify_resource_constraint(void)
{
	struct rc_common_state notify;
	struct constraint_status d_con;

	notify.act = MT_LPM_SMC_ACT_SET;
	notify.id = MT_RM_CONSTRAINT_ID_ALL;
	notify.type = CONSTRAINT_UPDATE_VALID;
	d_con.is_valid = MT_SPM_RC_VALID_FW;
	notify.value = (void *)&d_con;
	mt_lp_rm_do_update(-1, PLAT_RC_STATUS, &notify);
}

void spm_boot_init(void)
{
	/* switch ck_off/axi_26m control to SPM */
	mmio_setbits_32(CLK_SCP_CFG_0, SPM_CK_CONTROL_EN);
	mmio_clrsetbits_32(CLK_SCP_CFG_1, CLK_SCP_CFG_1_MASK,
			   CLK_SCP_CFG_1_SPM);

	plat_spm_lock_init();
	mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);
	mt_lp_resource_manager_register(&plat_mt8192_rm);
	spm_notify_resource_constraint();
	mt_lp_resource_request_manager_register(&plat_mt8192_rq);
	mt_lp_resource_user_register("SPM", &spm_res_user);
	mt_spm_idle_generic_init();
	mt_spm_suspend_init();
}
void spm_set_bootaddr(unsigned long bootaddr)
{
	/* initialize core4~7 boot entry address */
	mmio_write_32(SW2SPM_MAILBOX_3, bootaddr);
}
