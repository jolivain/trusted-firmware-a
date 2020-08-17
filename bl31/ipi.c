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

void plat_ipi_cpu_stop(void)
{
	/*
	 * Execute PSCI CPU power off sequence which ultimately
	 * execute CPU shutdown sequence as well.
	 */
	psci_cpu_off();
}
void ipi_send_cpu_stop(u_register_t target)
{
	plat_ic_raise_el3_sgi(PLAT_IPI_CPU_STOP, target);
}
#endif

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
		ERROR("Unknown IPI 0x%x\n", id);
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
