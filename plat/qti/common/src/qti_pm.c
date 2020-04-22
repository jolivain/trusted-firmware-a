/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform.h>
#include "qti_cpu.h"
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <qtiseclib_defs_plat.h>
#include <qtiseclib_interface.h>
#include <qtiseclib_cb_interface.h>
#include <common/debug.h>
#include "assert.h"

#define arm_make_pwrstate_lvl3(lvl3_state, lvl2_state, lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl3_state) << (ARM_LOCAL_PSTATE_WIDTH * 3)) | \
		arm_make_pwrstate_lvl2(lvl2_state, lvl1_state, lvl0_state, pwr_lvl, type))


#if ARM_RECOM_STATE_ID_ENC
const unsigned int arm_pm_idle_states[] = {
	arm_make_pwrstate_lvl0(QTI_LOCAL_STATE_OFF,
			ARM_PWR_LVL0, PSTATE_TYPE_POWERDOWN),
	arm_make_pwrstate_lvl0(QTI_LOCAL_STATE_DEEPOFF,
			ARM_PWR_LVL0, PSTATE_TYPE_POWERDOWN),
	arm_make_pwrstate_lvl1(QTI_LOCAL_STATE_DEEPOFF,
			QTI_LOCAL_STATE_DEEPOFF,
			ARM_PWR_LVL1, PSTATE_TYPE_POWERDOWN),
	arm_make_pwrstate_lvl2(QTI_LOCAL_STATE_OFF,
			QTI_LOCAL_STATE_DEEPOFF,
			QTI_LOCAL_STATE_DEEPOFF,
			ARM_PWR_LVL2, PSTATE_TYPE_POWERDOWN),
	arm_make_pwrstate_lvl3(QTI_LOCAL_STATE_OFF,
			QTI_LOCAL_STATE_DEEPOFF,
			QTI_LOCAL_STATE_DEEPOFF,
			QTI_LOCAL_STATE_DEEPOFF,
			ARM_PWR_LVL3, PSTATE_TYPE_POWERDOWN),
	0,
};

/*******************************************************************************
 * ARM standard platform handler called to check the validity of the power
 * state parameter. The power state parameter has to be a composite power
 * state.
 ******************************************************************************/
int arm_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	unsigned int state_id;
	int i;

	assert(req_state);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; !!arm_pm_idle_states[i]; i++) {
		if (power_state == arm_pm_idle_states[i])
			break;
	}

	/* Return error if entry not found in the idle state array */
	if (!arm_pm_idle_states[i])
		return PSCI_E_INVALID_PARAMS;

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] = state_id &
						ARM_LOCAL_PSTATE_MASK;
		state_id >>= ARM_LOCAL_PSTATE_WIDTH;
	}

	return PSCI_E_SUCCESS;
}
#else
/*******************************************************************************
 * ARM standard platform handler called to check the validity of the power state
 * parameter.
 ******************************************************************************/
int arm_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	return PSCI_E_NOT_SUPPORTED;
}
#endif

/*******************************************************************************
 * PLATFORM FUNCTIONS
 ******************************************************************************/

void qti_set_cpupwrctlr_val(const psci_power_state_t *target_state, bool enter)
{
	unsigned long val;

        __asm__ volatile("mrs %[res], S3_0_C15_C2_7": [res] "=r" (val));

	if ((target_state->pwr_domain_state[ARM_PWR_LVL0] == QTI_LOCAL_STATE_OFF) ||
		(target_state->pwr_domain_state[ARM_PWR_LVL0] == QTI_LOCAL_STATE_DEEPOFF))
        {
		if (enter)
		{
			val |= CORE_PWRDN_EN_MASK;
                        qtiseclib_cb_gic_cpuif_disable();
		}
		else
		{
			val &= ~CORE_PWRDN_EN_MASK;
                        qtiseclib_cb_gic_cpuif_enable();
		}
	}

	__asm__ volatile("msr S3_0_C15_C2_7, %[res]" : : [res] "r" (val));
	isb();
}

/**
 * CPU power on function - ideally we want a wrapper since this function is
 * target specific. But to unblock teams.
 */
static int qti_cpu_power_on(u_register_t mpidr)
{
	int core_pos = plat_core_pos_by_mpidr(mpidr);

	/* If not valid mpidr, return error */
	if (core_pos < 0 || core_pos >= QTISECLIB_PLAT_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}

	return qtiseclib_psci_node_power_on(mpidr);
}

static void qti_cpu_power_on_finish(const psci_power_state_t * target_state)
{
	const uint8_t *pwr_states =
	    (const uint8_t *)target_state->pwr_domain_state;
	qtiseclib_psci_node_on_finish(pwr_states);
	qti_set_cpupwrctlr_val(target_state, false);
}

static void qti_cpu_standby(plat_local_state_t cpu_state)
{
}

static void qti_node_power_off(const psci_power_state_t * target_state)
{
	qtiseclib_psci_node_power_off((const uint8_t *)
				       target_state->pwr_domain_state);
	qti_set_cpupwrctlr_val(target_state, true);
}

static void qti_node_suspend(const psci_power_state_t * target_state)
{
	qtiseclib_psci_node_suspend((const uint8_t *)target_state->pwr_domain_state);
	qti_set_cpupwrctlr_val(target_state, true);
}

static void qti_node_suspend_finish(const psci_power_state_t * target_state)
{
	const uint8_t *pwr_states =
	    (const uint8_t *)target_state->pwr_domain_state;
	qtiseclib_psci_node_suspend_finish(pwr_states);
	qti_set_cpupwrctlr_val(target_state, false);
}

__dead2 void qti_domain_power_down_wfi(const psci_power_state_t * target_state)
{
	/* CPU specific cache maintenance before collapse. */
	qti_cpu_cm_at_pc(target_state->pwr_domain_state[1]);

	/* For now just do WFI - add any target specific handling if needed */
#ifdef ENABLE_CLUSTER_COHERENCY
	qtiseclib_disable_cluster_coherency(target_state->pwr_domain_state[1]);
#endif
	__asm volatile ("dsb sy \n"
			"wfi");

	/* We should never reach here */
	ERROR("PSCI: WFI fell through during power down (%d)",
	      plat_my_core_pos());
	panic();
}

__dead2 void qti_system_off(void)
{
	qtiseclib_psci_system_off();
}

__dead2 void qti_system_reset(void)
{
	qtiseclib_psci_system_reset();
}

void qti_get_sys_suspend_power_state(psci_power_state_t * req_state)
{
	int i = 0;
	unsigned int state_id, power_state;
	int size = sizeof(arm_pm_idle_states) / sizeof(arm_pm_idle_states[0]);

	/* Find deepest state */
	power_state = arm_pm_idle_states[size - 2];
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] = state_id & ARM_LOCAL_PSTATE_MASK;
		state_id >>= ARM_LOCAL_PSTATE_WIDTH;
	}
}
/* Structure containing platform specific PSCI operations. Common
 * PSCI layer will use this. */
const plat_psci_ops_t plat_qti_psci_pm_ops = {
	.pwr_domain_on = qti_cpu_power_on,
	.pwr_domain_on_finish = qti_cpu_power_on_finish,
	.cpu_standby = qti_cpu_standby,
	.pwr_domain_off = qti_node_power_off,
	.pwr_domain_suspend = qti_node_suspend,
	.pwr_domain_suspend_finish = qti_node_suspend_finish,
	.pwr_domain_pwr_down_wfi = qti_domain_power_down_wfi,
	.system_off = qti_system_off,
	.system_reset = qti_system_reset,
	.get_node_hw_state = NULL,
	.translate_power_state_by_mpidr = NULL,
	.get_sys_suspend_power_state = qti_get_sys_suspend_power_state,
	.validate_power_state = arm_validate_power_state,
};

/**
 * The ARM Standard platform definition of platform porting API
 * `plat_setup_psci_ops`.
 */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t ** psci_ops)
{
	int err;

	err = qtiseclib_psci_init();
	if (err == PSCI_E_SUCCESS) {
		*psci_ops = &plat_qti_psci_pm_ops;
	}

	return err;
}
