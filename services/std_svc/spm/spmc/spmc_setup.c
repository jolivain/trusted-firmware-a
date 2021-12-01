/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2021, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <context.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <lib/utils.h>
#include <platform_def.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <services/ffa_svc.h>

#include "spm_common.h"
#include "spmc.h"


unsigned int get_ec_index(sp_desc_t *sp)
{
	return plat_my_core_pos();
}

/* SEL1 partition specific initialisation. */
void spmc_el1_sp_setup(sp_desc_t *sp, entry_point_info_t *ep_info)
{
	/* Sanity check input arguments */
	assert(sp != NULL);
	assert(ep_info != NULL);

	/*
	 * Lets just zero the general purpose registers for now. This would be a
	 * good time to let the platform enforce its own boot protocol.
	 */
	zeromem(&ep_info->args, sizeof(ep_info->args));

	/*
	 * Check whether setup is being performed for the primary or a secondary
	 * execution context. In the latter case, indicate to the SP that this
	 * is a warm boot.
	 * TODO: This check would need to be reworked if the same entry point is
	 * used for both primary and secondary initialisation.
	 */
	if (sp->secondary_ep) {
		/*
		 * Sanity check that the secondary entry point is still what was
		 * originally set.
		 */
		assert(sp->secondary_ep == ep_info->pc);

		write_ctx_reg(get_gpregs_ctx(&sp->ec[get_ec_index(sp)].cpu_ctx),
			      CTX_GPREG_X0,
			      FFA_WB_TYPE_S2RAM);
	}
}

/* Common initialisation for all SPs. */
void spmc_sp_common_setup(sp_desc_t *sp, entry_point_info_t *ep_info)
{
	cpu_context_t *cpu_ctx;
	uint16_t sp_id;

	/* Assign FFA Partition ID if not already assigned */
	if (sp->sp_id == INV_SP_ID) {
		sp_id = FFA_SP_ID_BASE + ACTIVE_SP_DESC_INDEX;
		/* Ensure we don't clash with previously assigned partition IDs. */
		while (validate_partition_id(sp_id)) {
			sp_id++;
		}
		sp->sp_id = FFA_SP_ID_BASE + ACTIVE_SP_DESC_INDEX;
	}

	/* We currently only support S-EL1 partitions so ensure this is the case. */
	assert(sp->runtime_el == EL1);

	/*
	 * The initialisation of the SPSR in the ep_info should ideally be done
	 * in the EL specific initialisation routines above. However,
	 * cm_context_setup() needs this information to initialise system
	 * registers correctly. So, lets do this here.
	 */
	ep_info->spsr =	SPSR_64(MODE_EL1, MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS);

	/*
	 * Initialise the SP context based upon the entrypoint information
	 * collected so far. We are assuming that the index of the execution
	 * context used is the linear index of the primary cpu.
	 */
	cpu_ctx = &sp->ec[plat_my_core_pos()].cpu_ctx;
	print_entry_point_info(ep_info);
	cm_setup_context(cpu_ctx, ep_info);
}
