/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <common/debug.h>
#include <denver.h>
#include <mce.h>
#include <mce_private.h>
#include <plat/common/platform.h>
#include <lib/psci/psci.h>
#include <se.h>
#include <smmu.h>
#include <stdbool.h>
#include <string.h>
#include <t194_nvg.h>
#include <tegra194_private.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/* state id mask */
#define TEGRA194_STATE_ID_MASK		0xFU
/* constants to get power state's wake time */
#define TEGRA194_WAKE_TIME_MASK		0x0FFFFFF0U
#define TEGRA194_WAKE_TIME_SHIFT	4U
/* default core wake mask for CPU_SUSPEND */
#define TEGRA194_CORE_WAKE_MASK		0x180cU

static struct t19x_psci_percpu_data {
	uint32_t wake_time;
} __aligned(CACHE_WRITEBACK_GRANULE) t19x_percpu_data[PLATFORM_CORE_COUNT];

/*
 * tegra_fake_system_suspend acts as a boolean var controlling whether
 * we are going to take fake system suspend code or normal system suspend code
 * path. This variable is set inside the sip call handlers, when the kernel
 * requests an SIP call to set the suspend debug flags.
 */
uint8_t tegra_fake_system_suspend = 0U;

int32_t tegra_soc_validate_power_state(uint32_t power_state,
					psci_power_state_t *req_state)
{
	uint8_t state_id = (uint8_t)psci_get_pstate_id(power_state) & TEGRA194_STATE_ID_MASK;
	uint32_t cpu = plat_my_core_pos();
	int32_t ret = PSCI_E_SUCCESS;

	/* save the core wake time (in TSC ticks)*/
	t19x_percpu_data[cpu].wake_time = (power_state & TEGRA194_WAKE_TIME_MASK)
			<< TEGRA194_WAKE_TIME_SHIFT;

	/*
	 * Clean percpu_data[cpu] to DRAM. This needs to be done to ensure that
	 * the correct value is read in tegra_soc_pwr_domain_suspend(), which
	 * is called with caches disabled. It is possible to read a stale value
	 * from DRAM in that function, because the L2 cache is not flushed
	 * unless the cluster is entering CC6/CC7.
	 */
	clean_dcache_range((uint64_t)&t19x_percpu_data[cpu],
			sizeof(t19x_percpu_data[cpu]));

	/* Sanity check the requested state id */
	switch (state_id) {
	case PSTATE_ID_CORE_IDLE:
	case PSTATE_ID_CORE_POWERDN:

		/* Core powerdown request */
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = state_id;
		req_state->pwr_domain_state[MPIDR_AFFLVL1] = state_id;

		break;

	default:
		ERROR("%s: unsupported state id (%d)\n", __func__, state_id);
		ret = PSCI_E_INVALID_PARAMS;
		break;
	}

	return ret;
}

int32_t tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state;
	uint8_t stateid_afflvl0, stateid_afflvl2;
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t smmu_ctx_base;
	uint32_t val;
	mce_cstate_info_t cstate_info = { 0 };
	uint32_t cpu = plat_my_core_pos();
	int32_t ret;

	/* get the state ID */
	pwr_domain_state = target_state->pwr_domain_state;
	stateid_afflvl0 = pwr_domain_state[MPIDR_AFFLVL0] &
		TEGRA194_STATE_ID_MASK;
	stateid_afflvl2 = pwr_domain_state[PLAT_MAX_PWR_LVL] &
		TEGRA194_STATE_ID_MASK;

	if ((stateid_afflvl0 == PSTATE_ID_CORE_IDLE) ||
	    (stateid_afflvl0 == PSTATE_ID_CORE_POWERDN)) {

		/* Enter CPU idle/powerdown */
		val = (stateid_afflvl0 == PSTATE_ID_CORE_IDLE) ?
			(uint32_t)TEGRA_NVG_CORE_C6 : (uint32_t)TEGRA_NVG_CORE_C7;
		(void)mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE, (uint64_t)val,
				t19x_percpu_data[cpu].wake_time, 0U);

	} else if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {

		/* save 'Secure Boot' Processor Feature Config Register */
		val = mmio_read_32(TEGRA_MISC_BASE + MISCREG_PFCFG);
		mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_SECURE_BOOTP_FCFG, val);

		/* save SMMU context */
		smmu_ctx_base = params_from_bl2->tzdram_base +
				tegra194_get_smmu_ctx_offset();
		tegra_smmu_save_context((uintptr_t)smmu_ctx_base);

		/*
		 * Suspend SE, RNG1 and PKA1 only on silcon and fpga,
		 * since VDK does not support atomic se ctx save
		 */
		if (tegra_platform_is_silicon() || tegra_platform_is_fpga()) {
			ret = tegra_se_suspend();
			assert(ret == 0);
		}

		if (tegra_fake_system_suspend == false) {

			/* Prepare for system suspend */
			cstate_info.cluster = (uint32_t)TEGRA_NVG_CLUSTER_CC6;
			cstate_info.system = (uint32_t)TEGRA_NVG_SYSTEM_SC7;
			cstate_info.system_state_force = 1;
			cstate_info.update_wake_mask = 1;
			mce_update_cstate_info(&cstate_info);

			do {
				val = (uint32_t)mce_command_handler(
						(uint32_t)MCE_CMD_IS_SC7_ALLOWED,
						(uint32_t)TEGRA_NVG_CORE_C7,
						MCE_CORE_SLEEP_TIME_INFINITE,
						0U);
			} while (val == 0U);

			/* Instruct the MCE to enter system suspend state */
			(void)mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE,
					(uint64_t)TEGRA_NVG_CORE_C7,
					MCE_CORE_SLEEP_TIME_INFINITE, 0U);

			/* set system suspend state for house-keeping */
			tegra194_set_system_suspend_entry();
		}
	} else {
		; /* do nothing */
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Helper function to check if this is the last ON CPU in the cluster
 ******************************************************************************/
static bool tegra_last_on_cpu_in_cluster(const plat_local_state_t *states,
			uint32_t ncpu)
{
	plat_local_state_t target;
	bool last_on_cpu = true;
	uint32_t num_cpus = ncpu, pos = 0;

	do {
		target = states[pos];
		if (target != PLAT_MAX_OFF_STATE) {
			last_on_cpu = false;
		}
		--num_cpus;
		pos++;
	} while (num_cpus != 0U);

	return last_on_cpu;
}

/*******************************************************************************
 * Helper function to get target power state for the cluster
 ******************************************************************************/
static plat_local_state_t tegra_get_afflvl1_pwr_state(const plat_local_state_t *states,
			uint32_t ncpu)
{
	uint32_t core_pos = (uint32_t)read_mpidr() & (uint32_t)MPIDR_CPU_MASK;
	plat_local_state_t target = states[core_pos];
	mce_cstate_info_t cstate_info = { 0 };

	/* CPU suspend */
	if (target == PSTATE_ID_CORE_POWERDN) {

		/* Program default wake mask */
		cstate_info.wake_mask = TEGRA194_CORE_WAKE_MASK;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);
	}

	/* CPU off */
	if (target == PLAT_MAX_OFF_STATE) {

		/* Enable cluster powerdn from last CPU in the cluster */
		if (tegra_last_on_cpu_in_cluster(states, ncpu)) {

			/* Enable CC6 state and turn off wake mask */
			cstate_info.cluster = (uint32_t)TEGRA_NVG_CLUSTER_CC6;
			cstate_info.update_wake_mask = 1;
			mce_update_cstate_info(&cstate_info);

		} else {

			/* Turn off wake_mask */
			cstate_info.update_wake_mask = 1;
			mce_update_cstate_info(&cstate_info);
			target = PSCI_LOCAL_STATE_RUN;
		}
	}

	return target;
}

/*******************************************************************************
 * Platform handler to calculate the proper target power level at the
 * specified affinity level
 ******************************************************************************/
plat_local_state_t tegra_soc_get_target_pwr_state(uint32_t lvl,
					     const plat_local_state_t *states,
					     uint32_t ncpu)
{
	plat_local_state_t target = PSCI_LOCAL_STATE_RUN;
	uint32_t cpu = plat_my_core_pos();

	/* System Suspend */
	if ((lvl == (uint32_t)MPIDR_AFFLVL2) && (states[cpu] == PSTATE_ID_SOC_POWERDN)) {
		target = PSTATE_ID_SOC_POWERDN;
	}

	/* CPU off, CPU suspend */
	if (lvl == (uint32_t)MPIDR_AFFLVL1) {
		target = tegra_get_afflvl1_pwr_state(states, ncpu);
	}

	/* target cluster/system state */
	return target;
}

int32_t tegra_soc_pwr_domain_power_down_wfi(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state =
		target_state->pwr_domain_state;
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint8_t stateid_afflvl2 = pwr_domain_state[PLAT_MAX_PWR_LVL] &
		TEGRA194_STATE_ID_MASK;
	uint64_t val;
	u_register_t ns_sctlr_el1;

	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {
		/*
		 * The TZRAM loses power when we enter system suspend. To
		 * allow graceful exit from system suspend, we need to copy
		 * BL3-1 over to TZDRAM.
		 */
		val = params_from_bl2->tzdram_base +
		      tegra194_get_cpu_reset_handler_size();
		memcpy((void *)(uintptr_t)val, (void *)(uintptr_t)BL31_BASE,
		       (uintptr_t)&__BL31_END__ - (uintptr_t)BL31_BASE);

		/*
		 * In fake suspend mode, ensure that the loopback procedure
		 * towards system suspend exit is started, instead of calling
		 * WFI. This is done by disabling both MMU's of EL1 & El3
		 * and calling tegra_secure_entrypoint().
		 */
		if (tegra_fake_system_suspend) {

			/*
			 * Disable EL1's MMU.
			 */
			ns_sctlr_el1 = read_sctlr_el1();
			ns_sctlr_el1 &= (~((u_register_t)SCTLR_M_BIT));
			write_sctlr_el1(ns_sctlr_el1);
			isb();
			dsbsy();

			disable_mmu_el3();
			tegra_secure_entrypoint();
		}
	}

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_on(u_register_t mpidr)
{
	uint64_t target_cpu = mpidr & MPIDR_CPU_MASK;
	uint64_t target_cluster = (mpidr & MPIDR_CLUSTER_MASK) >>
			MPIDR_AFFINITY_BITS;

	if (target_cluster > ((uint32_t)PLATFORM_CLUSTER_COUNT - 1U)) {
		ERROR("%s: unsupported CPU (0x%lx)\n", __func__ , mpidr);
		return PSCI_E_NOT_PRESENT;
	}

	/* construct the target CPU # */
	target_cpu += (target_cluster * 2U);

	(void)mce_command_handler((uint64_t)MCE_CMD_ONLINE_CORE, target_cpu, 0U, 0U);

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint8_t stateid_afflvl2 = target_state->pwr_domain_state[PLAT_MAX_PWR_LVL];

	/*
	 * Reset power state info for CPUs when onlining, we set
	 * deepest power when offlining a core but that may not be
	 * requested by non-secure sw which controls idle states. It
	 * will re-init this info from non-secure software when the
	 * core come online.
	 */

	/*
	 * Check if we are exiting from deep sleep and restore SE
	 * context if we are.
	 */
	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {

		/*
		 * Enable strict checking after programming the GSC for
		 * enabling TZSRAM and TZDRAM
		 */
		mce_enable_strict_checking();

		/* Init SMMU */
		tegra_smmu_init();

		/* Resume SE, RNG1 and PKA1 */
		tegra_se_resume();

		/*
		 * Reset power state info for the last core doing SC7
		 * entry and exit, we set deepest power state as CC7
		 * and SC7 for SC7 entry which may not be requested by
		 * non-secure SW which controls idle states.
		 */
	}

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

	(void)target_state;

	/* Disable Denver's DCO operations */
	if (impl == DENVER_IMPL) {
		denver_disable_dco();
	}

	/* Turn off CPU */
	(void)mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE,
			(uint64_t)TEGRA_NVG_CORE_C7, MCE_CORE_SLEEP_TIME_INFINITE, 0U);

	return PSCI_E_SUCCESS;
}

__dead2 void tegra_soc_prepare_system_off(void)
{
	/* System power off */

	/* SC8 */

	wfi();

	/* wait for the system to power down */
	for (;;) {
		;
	}
}

int32_t tegra_soc_prepare_system_reset(void)
{
	return PSCI_E_SUCCESS;
}
