/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* common headers */
#include <lib/psci/psci.h>

/* mediatek platform specific headers */
/* platform specific headers */
#include <plat_mtk_lpm.h>
#include <plat_pm.h>
#include <mt_gic_v3.h>
#include <mtk_brisket.h>
#include <mtk_credit_didt.h>
#include <mtspmc.h>
#include <plat/common/platform.h>
#include <plat_params.h>
#include <pmic.h>

/*
 * Cluster state request:
 * [0] : The CPU requires cluster power down
 * [1] : The CPU requires cluster power on
 */
#define coordinate_cluster(onoff)	write_clusterpwrdn_el1(onoff)
#define coordinate_cluster_pwron()	coordinate_cluster(1)
#define coordinate_cluster_pwroff()	coordinate_cluster(0)

/* platform secure entry point */
static uintptr_t secure_entrypoint;
/* per-CPU power state */
static unsigned int plat_power_state[PLATFORM_CORE_COUNT];

/* platform CPU power domain - ops */
static const struct mt_lpm_tz *plat_mt_pm;

#define plat_mt_pm_invoke(_name, _cpu, _state) ({ \
	int ret = -1; \
	if (plat_mt_pm != NULL && plat_mt_pm->_name != NULL) { \
		ret = plat_mt_pm->_name(_cpu, _state); \
	} \
	ret; })

#define plat_mt_pm_invoke_no_check(_name, _cpu, _state) ({ \
	if (plat_mt_pm != NULL && plat_mt_pm->_name != NULL) { \
		(void) plat_mt_pm->_name(_cpu, _state); \
	} \
	})

/*
 * Common MTK_platform operations to power on/off a
 * CPU in response to a CPU_ON, CPU_OFF or CPU_SUSPEND request.
 */

static void plat_cpu_pwrdwn_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	plat_mt_pm_invoke_no_check(pwr_cpu_dwn, cpu, state);

	if ((psci_get_pstate_pwrlvl(req_pstate) >= MTK_AFFLVL_CLUSTER) ||
			(req_pstate == 0U)) { /* hotplug off */
		coordinate_cluster_pwroff();
	}

	/* Prevent interrupts from spuriously waking up this CPU */
	mt_gic_rdistif_save();
	gicv3_cpuif_disable(plat_my_core_pos());
	gicv3_rdistif_off(plat_my_core_pos());

	/* Brisket config */
	brisket_init(0);
}

static void plat_cpu_pwron_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	plat_mt_pm_invoke_no_check(pwr_cpu_on, cpu, state);

	coordinate_cluster_pwron();

	/* Enable the GIC CPU interface */

	/* Brisket config */
	brisket_init(1);

	gicv3_rdistif_on(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
	mt_gic_rdistif_init();

	/*
	 * If mcusys do power down before then restore
	 * all CPUs' GIC Redistributors
	 */
	if (IS_MCUSYS_OFF_STATE(state)) {
		mt_gic_rdistif_restore_all();
	} else {
		mt_gic_rdistif_restore();
	}
	/* Credit config */
	credit_didt_init();
}

/*
 * Common MTK_platform operations to power on/off a
 * cluster in response to a CPU_ON, CPU_OFF or CPU_SUSPEND request.
 */

static void plat_cluster_pwrdwn_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	if (plat_mt_pm_invoke(pwr_cluster_dwn, cpu, state) != 0) {
		coordinate_cluster_pwron();

		/* TODO: return on fail.
		 *       Add a 'return' here before adding any code following
		 *       the if-block.
		 */
	}
}

static void plat_cluster_pwron_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	if (plat_mt_pm_invoke(pwr_cluster_on, cpu, state) != 0) {
		/* TODO: return on fail.
		 *       Add a 'return' here before adding any code following
		 *       the if-block.
		 */
	}
}

/*
 * Common MTK_platform operations to power on/off a
 * mcusys in response to a CPU_ON, CPU_OFF or CPU_SUSPEND request.
 */

static void plat_mcusys_pwrdwn_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	if (plat_mt_pm_invoke(pwr_mcusys_dwn, cpu, state) != 0) {
		return;		/* return on fail */
	}

	mt_gic_distif_save();
	gic_sgi_save_all();
}

static void plat_mcusys_pwron_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	if (plat_mt_pm_invoke(pwr_mcusys_on, cpu, state) != 0) {
		return;		/* return on fail */
	}

	mt_gic_init();
	mt_gic_distif_restore();
	gic_sgi_restore_all();

	plat_mt_pm_invoke_no_check(pwr_mcusys_on_finished, cpu, state);
}

/*
 * plat_psci_ops implementation
 */

static void plat_cpu_standby(plat_local_state_t cpu_state)
{
	uint64_t scr;

	scr = read_scr_el3();
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);

	isb();
	dsb();
	wfi();

	write_scr_el3(scr);
}

static int plat_power_domain_on(u_register_t mpidr)
{
	unsigned int cpu = (unsigned int)plat_core_pos_by_mpidr(mpidr);
	unsigned int cluster = 0U;

	if (cpu >= PLATFORM_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (spm_get_cluster_powerstate(cluster) == 0) {
		spm_poweron_cluster(cluster);
	}

	/* init CPU reset arch as AARCH64 */
	mcucfg_init_archstate(cluster, cpu, 1);
	mcucfg_set_bootaddr(cluster, cpu, secure_entrypoint);
	spm_poweron_cpu(cluster, cpu);

	return PSCI_E_SUCCESS;
}
/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static const plat_psci_ops_t plat_plat_pm_ops = {
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_plat_pm_ops;

	return 0;
}
