/*
 * Copyright (c) 2020 Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <bl31/ehf.h>
#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <lib/psci/psci.h>
#include <platform_def.h>
#include <plat/common/platform.h>

#if defined(PLAT_IPI_CPU_STOP)
#pragma weak plat_ipi_cpu_stop

/*
 * This is invoked by the target core when 'PLAT_IPI_CPU_STOP'
 * is received. Default implementation powers off the core.
 */
void plat_ipi_cpu_stop(void)
{
	/* Power down the core */
	psci_cpu_off();
}

/*
 * This API can be used to stop a core specified by 'MPIDR'.
 * 'target_mpidr' is the 'MPIDR' of the target core.
 *
 * In order to stop all other cores platform can provide this
 * api as the 'stop_func' to
 * 'psci_stop_other_cores(unsigned int wait_ms,
 *			  void (*stop_func)(u_register_t mpidr))'
 */
void ipi_send_cpu_stop(u_register_t target_mpidr)
{
	plat_ic_raise_el3_sgi(PLAT_IPI_CPU_STOP, target_mpidr);
}
#endif

/* 'EHF' handler for generic 'IPI's defined by 'PLAT_IPI_PRI' */
static int ipi_handler(unsigned int id, unsigned int flags,
		       void *handle, void *cookie)
{
	switch (id) {
#if defined(PLAT_IPI_CPU_STOP)
	case PLAT_IPI_CPU_STOP:
		plat_ipi_cpu_stop();
		break;
#endif
	default:
		WARN("Unknown IPI 0x%x\n", id);
		break;
	}

	plat_ic_end_of_interrupt(id);

	return 0;
}

__init void ipi_init(void)
{
#if defined(PLAT_IPI_PRI)
	ehf_register_priority_handler(PLAT_IPI_PRI, ipi_handler);
#endif
}
