/*
 * Copyright (c) 2013-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <arch_features.h>
#include <arch_helpers.h>
#include <bl32/tsp/tsp.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <platform_tsp.h>

#include <lib/psci/psci.h>
#include <services/ffa_svc.h>

#include "tsp_private.h"


/*******************************************************************************
 * Lock to control access to the console
 ******************************************************************************/
spinlock_t console_lock;

/*******************************************************************************
 * Per cpu data structure to populate parameters for an SMC in C code and use
 * a pointer to this structure in assembler code to populate x0-x7
 ******************************************************************************/
static tsp_args_t tsp_smc_args[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Per cpu data structure to keep track of TSP activity
 ******************************************************************************/
work_statistics_t tsp_stats[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * The TSP memory footprint starts at address BL32_BASE and ends with the
 * linker symbol __BL32_END__. Use these addresses to compute the TSP image
 * size.
 ******************************************************************************/
#define BL32_TOTAL_LIMIT BL32_END
#define BL32_TOTAL_SIZE (BL32_TOTAL_LIMIT - (unsigned long) BL32_BASE)

static unsigned int spmc_id;

tsp_args_t tsp_smc(uint32_t func, uint64_t arg0,
		   uint64_t arg1, uint64_t arg2,
		   uint64_t arg3, uint64_t arg4,
		   uint64_t arg5, uint64_t arg6)
{
	tsp_args_t ret_args = {0};

	register uint64_t r0 __asm__("x0") = func;
	register uint64_t r1 __asm__("x1") = arg0;
	register uint64_t r2 __asm__("x2") = arg1;
	register uint64_t r3 __asm__("x3") = arg2;
	register uint64_t r4 __asm__("x4") = arg3;
	register uint64_t r5 __asm__("x5") = arg4;
	register uint64_t r6 __asm__("x6") = arg5;
	register uint64_t r7 __asm__("x7") = arg6;

	/* Output registers, also used as inputs ('+' constraint). */
	__asm__ volatile("smc #0"
			: "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4),
			  "+r"(r5), "+r"(r6), "+r"(r7));

	ret_args._regs[0] = r0;
	ret_args._regs[1] = r1;
	ret_args._regs[2] = r2;
	ret_args._regs[3] = r3;
	ret_args._regs[4] = r4;
	ret_args._regs[5] = r5;
	ret_args._regs[6] = r6;
	ret_args._regs[7] = r7;

	return ret_args;
}

static tsp_args_t *set_smc_args(uint64_t arg0,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7)
{
	uint32_t linear_id;
	tsp_args_t *pcpu_smc_args;

	/*
	 * Return to Secure Monitor by raising an SMC. The results of the
	 * service are passed as an arguments to the SMC
	 */
	linear_id = plat_my_core_pos();
	pcpu_smc_args = &tsp_smc_args[linear_id];
	write_sp_arg(pcpu_smc_args, TSP_ARG0, arg0);
	write_sp_arg(pcpu_smc_args, TSP_ARG1, arg1);
	write_sp_arg(pcpu_smc_args, TSP_ARG2, arg2);
	write_sp_arg(pcpu_smc_args, TSP_ARG3, arg3);
	write_sp_arg(pcpu_smc_args, TSP_ARG4, arg4);
	write_sp_arg(pcpu_smc_args, TSP_ARG5, arg5);
	write_sp_arg(pcpu_smc_args, TSP_ARG6, arg6);
	write_sp_arg(pcpu_smc_args, TSP_ARG7, arg7);

	return pcpu_smc_args;
}

/*******************************************************************************
 * Setup function for TSP.
 ******************************************************************************/
void tsp_setup(void)
{
	/* Perform early platform-specific setup */
	tsp_early_platform_setup();

	/* Perform late platform-specific setup */
	tsp_plat_arch_setup();

#if ENABLE_PAUTH
	/*
	 * Assert that the ARMv8.3-PAuth registers are present or an access
	 * fault will be triggered when they are being saved or restored.
	 */
	assert(is_armv8_3_pauth_present());
#endif /* ENABLE_PAUTH */
}

static tsp_args_t *send_ffa_pm_success(void)
{
	uint32_t linear_id = plat_my_core_pos();
	unsigned int tsp_id;
	tsp_args_t smc_args = {0};

	/* Get the TSP ID */
	smc_args = tsp_smc(FFA_ID_GET, 0, 0, 0, 0, 0, 0, 0);
	if (smc_args._regs[TSP_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not get own ID (0x%lx) on core%d\n",
			smc_args._regs[2], linear_id);
		panic();
	}

	tsp_id = smc_args._regs[2];

	return set_smc_args(FFA_MSG_SEND_DIRECT_RESP_SMC32,
			    tsp_id << FFA_DIRECT_MSG_SOURCE_SHIFT |
			    spmc_id,
			    FFA_FWK_MSG_BIT |
			    (FFA_PM_MSG_PM_RESP & FFA_FWK_MSG_MASK),
			    0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * before this cpu is turned off in response to a psci cpu_off request.
 ******************************************************************************/
tsp_args_t *tsp_cpu_off_main(uint64_t arg0,
			     uint64_t arg1,
			     uint64_t arg2,
			     uint64_t arg3,
			     uint64_t arg4,
			     uint64_t arg5,
			     uint64_t arg6,
			     uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/*
	 * This cpu is being turned off, so disable the timer to prevent the
	 * secure timer interrupt from interfering with power down. A pending
	 * interrupt will be lost but we do not care as we are turning off.
	 */
	tsp_generic_timer_stop();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_off_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx off request\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu off requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_off_count);
	spin_unlock(&console_lock);
#endif

#if SPMC_AT_EL3
	return send_ffa_pm_success();
#else
	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_OFF_DONE, 0, 0, 0, 0, 0, 0, 0);
#endif
}

/*******************************************************************************
 * This function performs any book keeping in the test secure payload before
 * this cpu's architectural state is saved in response to an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
tsp_args_t *tsp_cpu_suspend_main(uint64_t arg0,
				 uint64_t arg1,
				 uint64_t arg2,
				 uint64_t arg3,
				 uint64_t arg4,
				 uint64_t arg5,
				 uint64_t arg6,
				 uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/*
	 * Save the time context and disable it to prevent the secure timer
	 * interrupt from interfering with wakeup from the suspend state.
	 */
	tsp_generic_timer_save();
	tsp_generic_timer_stop();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_suspend_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu suspend requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_suspend_count);
	spin_unlock(&console_lock);
#endif

#if SPMC_AT_EL3
	return send_ffa_pm_success();
#else
	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_SUSPEND_DONE, 0, 0, 0, 0, 0, 0, 0);
#endif
}

/*******************************************************************************
 * This function performs any book keeping in the test secure payload after this
 * cpu's architectural state has been restored after wakeup from an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
tsp_args_t *tsp_cpu_resume_main(uint64_t max_off_pwrlvl,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Restore the generic timer context */
	tsp_generic_timer_restore();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_resume_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx resumed. maximum off power level %" PRId64 "\n",
	     read_mpidr(), max_off_pwrlvl);
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu resume requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_resume_count);
	spin_unlock(&console_lock);
#endif

#if SPMC_AT_EL3
	return send_ffa_pm_success();
#else
	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_RESUME_DONE, 0, 0, 0, 0, 0, 0, 0);
#endif
}

/*******************************************************************************
 * This function performs any remaining bookkeeping in the test secure payload
 * before the system is switched off (in response to a psci SYSTEM_OFF request)
 ******************************************************************************/
tsp_args_t *tsp_system_off_main(uint64_t arg0,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx SYSTEM_OFF request\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets requests\n", read_mpidr(),
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count);
	spin_unlock(&console_lock);
#endif

	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_SYSTEM_OFF_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any remaining bookkeeping in the test secure payload
 * before the system is reset (in response to a psci SYSTEM_RESET request)
 ******************************************************************************/
tsp_args_t *tsp_system_reset_main(uint64_t arg0,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx SYSTEM_RESET request\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets requests\n", read_mpidr(),
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count);
	spin_unlock(&console_lock);
#endif

	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_SYSTEM_RESET_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * TSP fast smc handler. The secure monitor jumps to this function by
 * doing the ERET after populating X0-X7 registers. The arguments are received
 * in the function arguments in order. Once the service is rendered, this
 * function returns to Secure Monitor by raising SMC.
 ******************************************************************************/
tsp_args_t *tsp_smc_handler(uint64_t func,
			    uint64_t arg1,
			    uint64_t arg2,
			    uint64_t arg3,
			    uint64_t arg4,
			    uint64_t arg5,
			    uint64_t arg6,
			    uint64_t arg7)
{
	uint128_t service_args;
	uint64_t service_arg0;
	uint64_t service_arg1;
	uint64_t results[2];
	uint32_t linear_id = plat_my_core_pos();
	u_register_t dit;

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx received %s smc 0x%" PRIx64 "\n", read_mpidr(),
		((func >> 31) & 1) == 1 ? "fast" : "yielding",
		func);
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets\n", read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count);
	spin_unlock(&console_lock);
#endif

	/* Render secure services and obtain results here */
	results[0] = arg1;
	results[1] = arg2;

	/*
	 * Request a service back from dispatcher/secure monitor.
	 * This call returns and thereafter resumes execution.
	 */
	service_args = tsp_get_magic();
	service_arg0 = (uint64_t)service_args;
	service_arg1 = (uint64_t)(service_args >> 64U);

#if CTX_INCLUDE_MTE_REGS
	/*
	 * Write a dummy value to an MTE register, to simulate usage in the
	 * secure world
	 */
	write_gcr_el1(0x99);
#endif

	/* Determine the function to perform based on the function ID */
	switch (TSP_BARE_FID(func)) {
	case TSP_ADD:
		results[0] += service_arg0;
		results[1] += service_arg1;
		break;
	case TSP_SUB:
		results[0] -= service_arg0;
		results[1] -= service_arg1;
		break;
	case TSP_MUL:
		results[0] *= service_arg0;
		results[1] *= service_arg1;
		break;
	case TSP_DIV:
		results[0] /= service_arg0 ? service_arg0 : 1;
		results[1] /= service_arg1 ? service_arg1 : 1;
		break;
	case TSP_CHECK_DIT:
		if (!is_armv8_4_dit_present()) {
#if LOG_LEVEL >= LOG_LEVEL_ERROR
			spin_lock(&console_lock);
			ERROR("DIT not supported\n");
			spin_unlock(&console_lock);
#endif
			results[0] = 0;
			results[1] = 0xffff;
			break;
		}
		dit = read_dit();
		results[0] = dit == service_arg0;
		results[1] = dit;
		/* Toggle the dit bit */
		write_dit(service_arg0 != 0U ? 0 : DIT_BIT);
		break;
	default:
		break;
	}

	return set_smc_args(func, 0,
			    results[0],
			    results[1],
			    0, 0, 0, 0);
}

/*******************************************************************************
 * TSP smc abort handler. This function is called when aborting a preempted
 * yielding SMC request. It should cleanup all resources owned by the SMC
 * handler such as locks or dynamically allocated memory so following SMC
 * request are executed in a clean environment.
 ******************************************************************************/
tsp_args_t *tsp_abort_smc_handler(uint64_t func,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7)
{
	return set_smc_args(TSP_ABORT_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function handles framework messages. Currently only PM.
 ******************************************************************************/
static tsp_args_t *handle_framework_message(uint64_t arg0,
					    uint64_t arg1,
					    uint64_t arg2,
					    uint64_t arg3,
					    uint64_t arg4,
					    uint64_t arg5,
					    uint64_t arg6,
					    uint64_t arg7)
{
	/* Check if it is a power management message from the SPMC. */
	if (ffa_endpoint_source(arg1) != spmc_id)
		goto err;

	/* Check if it is a PM request message. */
	if ((arg2 & FFA_FWK_MSG_MASK) == FFA_FWK_MSG_PSCI) {
		/* Check if it is a PSCI CPU_OFF request. */
		if (arg3 == PSCI_CPU_OFF) {
			return tsp_cpu_off_main(arg0, arg1, arg2, arg3,
						arg4, arg5, arg6, arg7);
		} else if (arg3 == PSCI_CPU_SUSPEND_AARCH64) {
			return tsp_cpu_suspend_main(arg0, arg1, arg2, arg3,
						arg4, arg5, arg6, arg7);
		}
	} else if ((arg2 & FFA_FWK_MSG_MASK) == FFA_PM_MSG_WB_REQ) {
		/* Check it is a PSCI Warm Boot request. */
		if (arg3 == FFA_WB_TYPE_NOTS2RAM) {
			return tsp_cpu_resume_main(arg0, arg1, arg2, arg3,
						arg4, arg5, arg6, arg7);
		}
	}

err:
	ERROR("%s: Unknown framework message!\n", __func__);
	panic();
}

/*******************************************************************************
 * This function implements the event loop for handling FF-A ABI invocations.
 ******************************************************************************/
static tsp_args_t *tsp_event_loop(uint64_t smc_fid,
			   uint64_t arg1,
			   uint64_t arg2,
			   uint64_t arg3,
			   uint64_t arg4,
			   uint64_t arg5,
			   uint64_t arg6,
			   uint64_t arg7)
{
	/* Panic if the SPMC did not forward an FF-A call. */
	if (!is_ffa_fid(smc_fid)) {
		ERROR("%s: Unknown SMC FID (0x%lx)\n", __func__, smc_fid);
		panic();
	}

	switch (smc_fid) {
	case FFA_INTERRUPT:
		/*
		 * IRQs were enabled upon re-entry into the TSP. The interrupt
		 * must have been handled by now. Return to the SPMC indicating
		 * the same.
		 */
		return set_smc_args(FFA_MSG_WAIT, 0, 0, 0, 0, 0, 0, 0);

	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
		/* Check if a framework message, handle accordingly. */
		if ((arg2 & FFA_FWK_MSG_BIT)) {
			return handle_framework_message(smc_fid, arg1, arg2, arg3,
							arg4, arg5, arg6, arg7);
		}
	default:
		break;
	}

	ERROR("%s: Unsupported FF-A FID (0x%lx)\n", __func__, smc_fid);
	panic();
}

static tsp_args_t *tsp_loop(tsp_args_t *args)
{
	do {
		/* --------------------------------------------
		 * Tell SPMD that we are done initialising.*
		 * Also mask FIQ interrupts to avoid preemption
		 * in case EL3 SPMC delegates an IRQ next or a
		 * managed exit. Lastly, unmask IRQs so that
		 * they can be handled immediately upon re-entry
		 *  ---------------------------------------------
		 */
		write_daifset(DAIF_FIQ_BIT);
		write_daifclr(DAIF_IRQ_BIT);
		register uint64_t r0 __asm__("x0") = args->_regs[0];
		register uint64_t r1 __asm__("x1") = args->_regs[1];
		register uint64_t r2 __asm__("x2") = args->_regs[2];
		register uint64_t r3 __asm__("x3") = args->_regs[3];
		register uint64_t r4 __asm__("x4") = args->_regs[4];
		register uint64_t r5 __asm__("x5") = args->_regs[5];
		register uint64_t r6 __asm__("x6") = args->_regs[6];
		register uint64_t r7 __asm__("x7") = args->_regs[7];
		/* Output registers, also used as inputs ('+' constraint). */
		__asm__ volatile("smc #0"
				: "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4),
				"+r"(r5), "+r"(r6), "+r"(r7));
		 args = tsp_event_loop(r0, r1, r2, r3, r4, r5, r6, r7);
	} while (1);

	/* Not Reached. */
	return NULL;
}

/*******************************************************************************
 * TSP main entry point where it gets the opportunity to initialize its secure
 * state/applications. Once the state is initialized, it must return to the
 * SPD with a pointer to the 'tsp_vector_table' jump table.
 ******************************************************************************/
uint64_t tsp_main(void)
{
	tsp_args_t smc_args = {0};

	NOTICE("TSP: %s\n", version_string);
	NOTICE("TSP: %s\n", build_message);
	INFO("TSP: Total memory base : 0x%lx\n", (unsigned long) BL32_BASE);
	INFO("TSP: Total memory size : 0x%lx bytes\n", BL32_TOTAL_SIZE);
	uint32_t linear_id = plat_my_core_pos();
	/* Initialize the platform */
	tsp_platform_setup();

	/* Initialize secure/applications state here */
	tsp_generic_timer_start();

	/* Register secondary entrypoint with the SPMC. */
	smc_args = tsp_smc(FFA_SECONDARY_EP_REGISTER_SMC64,
			(uint64_t) tsp_cpu_on_entry,
			0, 0, 0, 0, 0, 0);
	if (smc_args._regs[TSP_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not register secondary ep (0x%lx)\n",
				smc_args._regs[2]);
	}
	/* Get TSP's endpoint id */
	smc_args = tsp_smc(FFA_ID_GET, 0, 0, 0, 0, 0, 0, 0);
	if (smc_args._regs[TSP_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not get own ID (0x%lx) on core%d\n",
				smc_args._regs[2], linear_id);
		panic();
	}

	INFO("TSP FF-A endpoint id = 0x%lx\n", smc_args._regs[2]);
	/* Get the SPMC ID */
	smc_args = tsp_smc(FFA_SPM_ID_GET, 0, 0, 0, 0, 0, 0, 0);
	if (smc_args._regs[TSP_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not get SPMC ID (0x%lx) on core%d\n",
				smc_args._regs[2], linear_id);
		panic();
	}

	spmc_id = smc_args._regs[2];

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu on requests\n",
			read_mpidr(),
			tsp_stats[linear_id].smc_count,
			tsp_stats[linear_id].eret_count,
			tsp_stats[linear_id].cpu_on_count);
	spin_unlock(&console_lock);
#endif
	return (uint64_t) tsp_loop(set_smc_args(FFA_MSG_WAIT, 0, 0, 0, 0, 0, 0, 0));
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * after this cpu's architectural state has been setup in response to an earlier
 * psci cpu_on request.
 ******************************************************************************/
tsp_args_t *tsp_cpu_on_main(void)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Initialize secure/applications state here */
	tsp_generic_timer_start();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;
#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx turned on\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu on requests\n",
			read_mpidr(),
			tsp_stats[linear_id].smc_count,
			tsp_stats[linear_id].eret_count,
			tsp_stats[linear_id].cpu_on_count);
	spin_unlock(&console_lock);
#endif
	/* ---------------------------------------------
	 * Jump to the main event loop to return to EL3
	 * and be ready for the next request on this cpu
	 * ---------------------------------------------
	 */
	return tsp_loop(set_smc_args(FFA_MSG_WAIT, 0, 0, 0, 0, 0, 0, 0));
}
