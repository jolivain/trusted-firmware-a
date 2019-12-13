/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/*******************************************************************************
 * Turn ON snoop control unit. This is needed to synchronize the data between
 * CPU's.
 ******************************************************************************/
static void a5ds_enable_scu(void)
{
	uint32_t scu_ctrl = mmio_read_32(A5DS_SCU_BASE + A5DS_SCU_CTRL);

	/* already enabled? */
	if (scu_ctrl & A5DS_SCU_ENABLE)
		return;

	scu_ctrl |= A5DS_SCU_ENABLE;
	mmio_write_32(A5DS_SCU_BASE + A5DS_SCU_CTRL, scu_ctrl);
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
static int a5ds_pwr_domain_on(u_register_t mpidr)
{
	unsigned int pos = plat_core_pos_by_mpidr(mpidr);
	uint64_t *hold_base = (uint64_t *)A5DS_HOLD_BASE;

	hold_base[pos] = A5DS_HOLD_STATE_GO;
	dsbish();
	sev();

	/*enable snoop control unit */
	a5ds_enable_scu();

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void a5ds_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* TODO: This setup is needed only after a cold boot*/
	gicv2_pcpu_distif_init();

	/* Enable the gic cpu interface */
	gicv2_cpuif_enable();
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 * a5ds only has always-on power domain and there is no power control present.
 ******************************************************************************/
void a5ds_pwr_domain_off(const psci_power_state_t *target_state)
{
	ERROR("CPU_OFF not supported on this platform\n");
	assert(false);
	panic();
}

/*******************************************************************************
 * Export the platform handlers via a5ds_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/
plat_psci_ops_t a5ds_psci_pm_ops = {
	/* dummy struct */
	.validate_ns_entrypoint = NULL,
	.pwr_domain_on = a5ds_pwr_domain_on,
	.pwr_domain_on_finish = a5ds_pwr_domain_on_finish,
	.pwr_domain_off = a5ds_pwr_domain_off
};

int __init plat_setup_psci_ops(uintptr_t sec_entrypoint,
				const plat_psci_ops_t **psci_ops)
{
	uintptr_t *mailbox = (void *)A5DS_TRUSTED_MAILBOX_BASE;
	*mailbox = sec_entrypoint;

	*psci_ops = &a5ds_psci_pm_ops;

	return 0;
}
