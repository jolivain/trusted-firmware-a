/*
 * Copyright (C) 2019 Repk repk@triplefau.lt
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <inttypes.h>
#include <stdint.h>

#include <common/bl_common.h>
#include <context.h>
#include <common/debug.h>
#include <arch_helpers.h>
#include <plat/common/platform.h>
#include <bl31/ea_handle.h>
#include <lib/el3_runtime/context_mgmt.h>

/*
 * This source file with custom plat_ea_handler function is compiled only when
 * building TF-A with compile option HANDLE_EA_EL3_FIRST_NS=1
 */
void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
#if PLATFORM_TEST_LOWER_EL_EA
	u_register_t elr_el3;
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	el3_state_t *state = get_el3state_ctx(ctx);


	unsigned int level = (unsigned int)GET_EL(read_spsr_el3());

	if (level < MODE_EL3) {
		if (ea_reason == ERROR_EA_SYNC) {
			INFO("Recieved sync EA from lower EL\n");
			elr_el3 = read_ctx_reg(state, CTX_ELR_EL3);
			elr_el3 += 4;
			write_ctx_reg(state, CTX_ELR_EL3, elr_el3);
			return;
		} else if (ea_reason == ERROR_EA_ASYNC) {
			INFO("Recieved Serror from lower EL\n");
			return;
		}
	}
#endif
	plat_default_ea_handler(ea_reason, syndrome, cookie, handle, flags);
}
