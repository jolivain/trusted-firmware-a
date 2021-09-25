/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <arch.h>
#include <arch_features.h>
#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>
#include <lib/extensions/mpam.h>
#include <lib/extensions/spe.h>
#include <lib/extensions/sve.h>
#include <lib/extensions/twed.h>
#include <lib/utils.h>

#include <fvp_r_arch_helpers.h>
#include <platform_def.h>


/*******************************************************************************
 * File contains EL2 equivalents of EL3 functions from
 * .../lib/el3_runtime/aarch64/context_mgmt.c
 ******************************************************************************/

/*******************************************************************************
 * Prepare the CPU system registers for first entry into secure or normal world
 *
 * The majority of the work needed is only for switching to non-secure, which
 * is not available on v8-R64 cores, so this function is very simple.
 ******************************************************************************/
void cm_prepare_el2_exit(uint32_t security_state)
{
	cm_el1_sysregs_context_restore(security_state);
	cm_set_next_eret_context(security_state);
}
