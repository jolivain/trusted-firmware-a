/*
 * Copyright (c) 2014-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/console.h>
#if RAS_EXTENSION
#include <lib/extensions/ras.h>
#endif
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <plat/common/platform.h>

#if CRASH_REPORTING
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#endif

/*
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that may be re-used by multiple
 * platforms but may also be overridden by a platform if required.
 */
#pragma weak bl31_plat_runtime_setup

#if SDEI_SUPPORT
#pragma weak plat_sdei_handle_masked_trigger
#pragma weak plat_sdei_validate_entry_point
#endif

#pragma weak plat_ea_handler = plat_default_ea_handler

void bl31_plat_runtime_setup(void)
{
	console_switch_state(CONSOLE_FLAG_RUNTIME);
}

/*
 * Helper function for platform_get_pos() when platform compatibility is
 * disabled. This is to enable SPDs using the older platform API to continue
 * to work.
 */
unsigned int platform_core_pos_helper(unsigned long mpidr)
{
	int idx = plat_core_pos_by_mpidr(mpidr);
	assert(idx >= 0);
	return idx;
}

#if SDEI_SUPPORT
/*
 * Function that handles spurious SDEI interrupts while events are masked.
 */
void plat_sdei_handle_masked_trigger(uint64_t mpidr, unsigned int intr)
{
	WARN("Spurious SDEI interrupt %u on masked PE %" PRIx64 "\n", intr, mpidr);
}

/*
 * Default Function to validate SDEI entry point, which returns success.
 * Platforms may override this with their own validation mechanism.
 */
int plat_sdei_validate_entry_point(uintptr_t ep, unsigned int client_mode)
{
	return 0;
}
#endif

const char *get_el_str(unsigned int el)
{
	if (el == MODE_EL3) {
		return "EL3";
	} else if (el == MODE_EL2) {
		return "EL2";
	}
	return "EL1";
}

#if CRASH_REPORTING
/*******************************************************************************
 * This function is used to dump contents of GP Regs which are context saved.
 ******************************************************************************/
void context_dump_gpregs(uint32_t security_state)
{
	cpu_context_t *ctx = cm_get_context(security_state);

	printf("\nContext Saved General Purpose Registers\n");

	printf("x0\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X0));
	printf("x1\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X1));
	printf("x2\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X2));
	printf("x3\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X3));
	printf("x4\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X4));
	printf("x5\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X5));
	printf("x6\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X6));
	printf("x7\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X7));
	printf("x8\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X8));
	printf("x9\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X9));
	printf("x10\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X10));
	printf("x11\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X11));
	printf("x12\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X12));
	printf("x13\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X13));
	printf("x14\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X14));
	printf("x15\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X15));
	printf("x16\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X16));
	printf("x17\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X17));
	printf("x18\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X18));
	printf("x19\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X19));
	printf("x20\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X20));
	printf("x21\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X21));
	printf("x22\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X22));
	printf("x23\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X23));
	printf("x24\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X24));
	printf("x25\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X25));
	printf("x26\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X26));
	printf("x27\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X27));
	printf("x28\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X28));
	printf("x29\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X29));
	printf("GPREG_LR\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_LR));
	printf("GPREG_SP_EL0\t = 0x%016" PRIx64 "\n",read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_SP_EL0));

	console_flush();
}
#endif


/* RAS functions common to AArch64 ARM platforms */
void plat_default_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
#if RAS_EXTENSION
	/* Call RAS EA handler */
	int handled = ras_ea_handler(ea_reason, syndrome, cookie, handle, flags);
	if (handled != 0)
		return;
#endif
	ERROR_NL();
	ERROR("Unhandled External Abort received on 0x%lx \n", read_mpidr_el1());
	ERROR("exception reason=%u syndrome=0x%" PRIx64 "\n", ea_reason, syndrome);

#if CRASH_REPORTING
	/* Assuming this the default plat_ea_handler and no handlers are registered
	 * and we are reaching here from a non secure world we could dump out context saved
	 * General Purpose register for debug purpose.
	 */
	context_dump_gpregs(NON_SECURE);
#endif
	panic();
}
