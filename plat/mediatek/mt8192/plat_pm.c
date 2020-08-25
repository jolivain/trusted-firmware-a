/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* common headers */
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/gpio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

/* mediatek platform specific headers */
#include <mt_gic_v3.h>
#include <mtspmc.h>
#include <plat_params.h>
#include <plat_pm.h>
#include <plat_mtk_lpm.h>
#include <pmic.h>
#include <mtk_brisket.h>
#include <mtk_credit_didt.h>

/*
 * The cpu require to cluster power stattus
 * [0] : The cpu require cluster power down
 * [1] : The cpu require cluster power on
 */
#define coordinate_cluster(onoff)	write_clusterpwrdn_el1(onoff)
#define coordinate_cluster_pwron()	coordinate_cluster(1)
#define coordinate_cluster_pwroff()	coordinate_cluster(0)

/* platform secure entry point */
static uintptr_t secure_entrypoint;
/* per-cpu power state */
static unsigned int plat_power_state[PLATFORM_CORE_COUNT];

/* platform cpu power domain - ops */
static struct mt_lpm_tz plat_mt_pm;

#define plat_mt_pm_invoke(_name, _cpu, _state) ({\
	int ret = -1;\
	if (plat_mt_pm._name)\
		ret = plat_mt_pm._name(_cpu, _state);\
	ret; })

int plat_mt_pm_register(struct mt_lpm_tz *mt_lpm)
{
	if (!mt_lpm)
		return -1;

	memcpy(&plat_mt_pm, mt_lpm, sizeof(struct mt_lpm_tz));

	return 0;
}

/* Power domain control implementation */
/*
 * Function which implements the common
 * MTK_platform specific operations to power down a
 * cpu in response to a CPU_OFF or CPU_SUSPEND request.
 */
static void plat_cpu_pwrdwn_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	plat_mt_pm_invoke(pwr_cpu_dwn, cpu, state);

	if (psci_get_pstate_pwrlvl(req_pstate) >= MTK_AFFLVL_CLUSTER ||
			req_pstate == 0) /* hotplug off */
		coordinate_cluster_pwroff();

	/* Prevent interrupts from spuriously waking up this cpu */
	mt_gic_rdistif_save();
	gicv3_cpuif_disable(plat_my_core_pos());
	gicv3_rdistif_off(plat_my_core_pos());

	/* Brisket config */
	brisket_init(0);
}

static void plat_cpu_pwron_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	plat_mt_pm_invoke(pwr_cpu_on, cpu, state);

	coordinate_cluster_pwron();

	/* Brisket config */
	brisket_init(1);

	/* Enable the gic cpu interface */
	gicv3_rdistif_on(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
	mt_gic_rdistif_init();

	/*
	 * If mcusys do power down before then restore
	 * all cpus gic rdistributor
	 */
	if (IS_MCUSYS_OFF_STATE(state))
		mt_gic_rdistif_restore_all();
	else
		mt_gic_rdistif_restore();

	/* Credit config */
	credit_didt_init();
}

/*
 * Function which implements the common
 * MTK_platform specific operations to power down a
 * cluster in response to a CPU_OFF or CPU_SUSPEND request.
 */
static void plat_cluster_pwrdwn_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	if (plat_mt_pm_invoke(pwr_cluster_dwn, cpu, state))
		return;		/* return on fail */
}

/*
 * Function which implements the common
 * MTK_platform specific operations to power on a
 * cluster in response to a CPU_ON or CPU_SUSPEND request.
 */
static void plat_cluster_pwron_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	plat_mt_pm_invoke(pwr_cluster_on, cpu, state);

#if PLAT_DEBUG_READY
	/* setup circular buffer */
	circular_buffer_setup();

	/* setup sram delsel */
	sram_delsel_setup();
#endif

#if defined(MTK_CM_MGR) && !defined(MTK_FPGA_EARLY_PORTING)
	/* init cpu stall counter */
	init_cpu_stall_counter_all();
#endif
}

/*
 * Function which implements the common
 * MTK_platform specific operations to power down
 * mcusys in response to a CPU_OFF or CPU_SUSPEND request.
 */
static void plat_mcusys_pwrdwn_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	if (plat_mt_pm_invoke(pwr_mcusys_dwn, cpu, state))
		return;		/* return on fail */

	mt_gic_distif_save();
	gic_sgi_save_all();
}

/*
 * Function which implements the common
 * MTK_platform specific operations to power on
 * mcusys in response to a CPU_ON or CPU_SUSPEND request.
 */
static void plat_mcusys_pwron_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	if (plat_mt_pm_invoke(pwr_mcusys_on, cpu, state))
		return;		/* return on fail */

	mt_gic_init();
	mt_gic_distif_restore();
	gic_sgi_restore_all();

#if DFD_READY
	dfd_resume();
#endif
	plat_mt_pm_invoke(pwr_mcusys_on_finished, cpu, state);
}


/* psci platform operation implementation */
static void plat_cpu_standby(plat_local_state_t cpu_state)
{
	unsigned int scr;

	scr = read_scr_el3();
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);

	isb();
	dsb();
	wfi();

	write_scr_el3(scr);
}

static int plat_power_domain_on(u_register_t mpidr)
{
	int cpu = plat_core_pos_by_mpidr(mpidr);
	int cluster = 0;

	if (!spm_get_cluster_powerstate(cluster))
		spm_poweron_cluster(cluster);

	/* init cpu reset arch as AARCH64 */
	mcucfg_init_archstate(cluster, cpu, 1);
	mcucfg_set_bootaddr(cluster, cpu, secure_entrypoint);
	spm_poweron_cpu(cluster, cpu);

	return PSCI_E_SUCCESS;
}

static void plat_power_domain_on_finish(const psci_power_state_t *state)
{
	unsigned long mpidr = read_mpidr_el1();
	int cpu = plat_core_pos_by_mpidr(mpidr);

#if ILDO_ENABLE
	/* for init(only on system boot up) */
	ildo_init(cpu, 0);
#endif

	/* Allow IRQs to wakeup this core in IDLE flow */
	mcucfg_enable_gic_wakeup(0, cpu);

	if (IS_CLUSTER_OFF_STATE(state))
		plat_cluster_pwron_common(cpu, state, 0);

	plat_cpu_pwron_common(cpu, state, 0);
}

static void plat_power_domain_off(const psci_power_state_t *state)
{
	unsigned long mpidr = read_mpidr_el1();
	int cpu = plat_core_pos_by_mpidr(mpidr);

	plat_cpu_pwrdwn_common(cpu, state, 0);
	spm_poweroff_cpu(0, cpu);

	/* prevent unintended IRQs from waking up the hot-unplugged core */
	mcucfg_disable_gic_wakeup(0, cpu);

	if (IS_CLUSTER_OFF_STATE(state))
		plat_cluster_pwrdwn_common(cpu, state, 0);
}

static void plat_power_domain_suspend(const psci_power_state_t *state)
{
	int cpu = plat_my_core_pos();

	plat_mt_pm_invoke(pwr_prompt, cpu, state);

	/* Perform the common cpu specific operations */
	plat_cpu_pwrdwn_common(cpu, state, plat_power_state[cpu]);

	if (IS_CLUSTER_OFF_STATE(state)) {
		/* Perform the common cluster specific operations */
		plat_cluster_pwrdwn_common(cpu, state, plat_power_state[cpu]);
	}

	if (IS_MCUSYS_OFF_STATE(state)) {
		/* Perform the common mcusys specific operations */
		plat_mcusys_pwrdwn_common(cpu, state, plat_power_state[cpu]);
	}
}

static void plat_power_domain_suspend_finish(const psci_power_state_t *state)
{
	int cpu = plat_my_core_pos();

	if (IS_MCUSYS_OFF_STATE(state)) {
		/* Perform the common mcusys specific operations */
		plat_mcusys_pwron_common(cpu, state, plat_power_state[cpu]);
	}

	if (IS_CLUSTER_OFF_STATE(state)) {
		/* Perform the common cluster specific operations */
		plat_cluster_pwron_common(cpu, state, plat_power_state[cpu]);
	}

	/* Perform the common cpu specific operations */
	plat_cpu_pwron_common(cpu, state, plat_power_state[cpu]);

#if ILDO_ENABLE
	ildo_init(cpu, 1);
#endif
	plat_mt_pm_invoke(pwr_reflect, cpu, state);
}

static int plat_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int i;
	unsigned int pstate = psci_get_pstate_type(power_state);
	int aff_lvl = psci_get_pstate_pwrlvl(power_state);
	unsigned int my_core_pos = plat_my_core_pos();

	if (aff_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	if (pstate == PSTATE_TYPE_STANDBY)
		req_state->pwr_domain_state[0] = PLAT_MAX_RET_STATE;
	else {
		unsigned int pstate_id = psci_get_pstate_id(power_state);
		plat_local_state_t s = MTK_LOCAL_STATE_OFF;

		/* Use pstate_id to be power domain state */
		if (pstate_id > s)
			s = (plat_local_state_t)pstate_id;

		for (i = 0; i <= aff_lvl; i++)
			req_state->pwr_domain_state[i] = s;
	}

	plat_power_state[my_core_pos] = power_state;
	return PSCI_E_SUCCESS;
}

static void plat_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	int lv = 0;
	unsigned int my_core_pos = plat_my_core_pos();

	for (lv = PSCI_CPU_PWR_LVL; lv <= PLAT_MAX_PWR_LVL; lv++)
		req_state->pwr_domain_state[lv] = PLAT_MAX_OFF_STATE;

	plat_power_state[my_core_pos] =
			psci_make_powerstate(
				MT_PLAT_PWR_STATE_SYSTEM_SUSPEND,
				PSTATE_TYPE_POWERDOWN, PLAT_MAX_PWR_LVL);

	flush_dcache_range((uintptr_t)
			&plat_power_state[my_core_pos],
			sizeof(plat_power_state[my_core_pos]));
}

/*******************************************************************************
 * MTK handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 plat_mtk_system_off(void)
{
	INFO("MTK System Off\n");

	pmic_power_off();

	wfi();
	ERROR("MTK System Off: operation not handled.\n");
	panic();
}

static void __dead2 plat_mtk_system_reset(void)
{
	struct bl_aux_gpio_info *gpio_reset = plat_get_mtk_gpio_reset();

	INFO("MTK System Reset\n");

	gpio_set_value(gpio_reset->index, gpio_reset->polarity);

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static const plat_psci_ops_t plat_plat_pm_ops = {
	.system_reset			= plat_mtk_system_reset,
	.cpu_standby			= plat_cpu_standby,
	.pwr_domain_on			= plat_power_domain_on,
	.pwr_domain_on_finish		= plat_power_domain_on_finish,
	.pwr_domain_off			= plat_power_domain_off,
	.pwr_domain_suspend		= plat_power_domain_suspend,
	.pwr_domain_suspend_finish	= plat_power_domain_suspend_finish,
	.system_off			= plat_mtk_system_off,
	.validate_power_state		= plat_validate_power_state,
	.get_sys_suspend_power_state	= plat_get_sys_suspend_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_plat_pm_ops;
	secure_entrypoint = sec_entrypoint;

	/*
	 * init the warm reset config for boot CPU
	 * reset arch as AARCH64
	 * reset addr as function bl31_warm_entrypoint()
	 */
	mcucfg_init_archstate(0, 0, 1);
	mcucfg_set_bootaddr(0, 0, secure_entrypoint);

	spmc_init();
	mt_plat_cpu_pm_init();

	return 0;
}
