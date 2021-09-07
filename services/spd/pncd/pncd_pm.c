/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>

#include "pnc.h"
#include "pncd_private.h"


/*******************************************************************************
 * The target cpu is being turned on. Allow the PNCD/PNC to perform any actions
 * needed. Nothing at the moment.
 ******************************************************************************/
static void pncd_cpu_on_handler(u_register_t target_cpu)
{
	pnc_generic_timer_start();
}

/*******************************************************************************
 * This cpu is being turned off. Allow the PNCD/PNC to perform any actions
 * needed
 ******************************************************************************/
static int32_t pncd_cpu_off_handler(u_register_t unused)
{
	uint32_t linear_id = plat_my_core_pos();

	if (linear_id == 0) {
		pnc_generic_timer_stop();
	}
	return 0;
}

/*******************************************************************************
 * This cpu is being suspended. S-EL1 state must have been saved in the
 * resident cpu (mpidr format) if it is a UP/UP migratable PNC.
 ******************************************************************************/
static void pncd_cpu_suspend_handler(u_register_t unused)
{
	uint32_t linear_id = plat_my_core_pos();

	if (linear_id == 0) {
		/*
		 * Save the time context and disable it to prevent the secure timer
		 * interrupt from interfering with wakeup from the suspend state.
		 */
		pnc_generic_timer_save();
		pnc_generic_timer_stop();
	}
}

/*******************************************************************************
 * This cpu has been turned on. Enter the PNC to initialise S-EL1 and other bits
 * before passing control back to the Secure Monitor. Entry in S-El1 is done
 * after initialising minimal architectural state that guarantees safe
 * execution.
 ******************************************************************************/
static void pncd_cpu_on_finish_handler(u_register_t unused)
{
}

/*******************************************************************************
 * This cpu has resumed from suspend. The SPD saved the PNC context when it
 * completed the preceding suspend call. Use that context to program an entry
 * into the PNC to allow it to do any remaining book keeping
 ******************************************************************************/
static void pncd_cpu_suspend_finish_handler(u_register_t max_off_pwrlvl)
{
	uint32_t linear_id = plat_my_core_pos();
	/* Restore the generic timer context */
	if (linear_id == 0) {
		pnc_generic_timer_restore();
		pnc_generic_timer_start();
	}
}

/*******************************************************************************
 * Return the type of PNC the PNCD is dealing with. Report the current resident
 * cpu (mpidr format) if it is a UP/UP migratable PNC.
 ******************************************************************************/
static int32_t pncd_cpu_migrate_info(u_register_t *resident_cpu)
{
	*resident_cpu = 0;
	return PSCI_TOS_NOT_UP_MIG_CAP;
}

/*******************************************************************************
 * System is about to be switched off. Allow the PNCD/PNC to perform
 * any actions needed.
 ******************************************************************************/
static void pncd_system_off(void)
{
}

/*******************************************************************************
 * System is about to be reset. Allow the PNCD/PNC to perform
 * any actions needed.
 ******************************************************************************/
static void pncd_system_reset(void)
{
}

/*******************************************************************************
 * Structure populated by the PNC Dispatcher to be given a chance to perform any
 * PNC bookkeeping before PSCI executes a power mgmt.  operation.
 ******************************************************************************/
const spd_pm_ops_t pncd_pm = {
	.svc_on = pncd_cpu_on_handler,
	.svc_off = pncd_cpu_off_handler,
	.svc_suspend = pncd_cpu_suspend_handler,
	.svc_on_finish = pncd_cpu_on_finish_handler,
	.svc_suspend_finish = pncd_cpu_suspend_finish_handler,
	.svc_migrate = NULL,
	.svc_migrate_info = pncd_cpu_migrate_info,
	.svc_system_off = pncd_system_off,
	.svc_system_reset = pncd_system_reset
};
