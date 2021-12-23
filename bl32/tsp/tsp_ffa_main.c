/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
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
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/ffa_svc.h>

#include "tsp_private.h"
#include "ffa_helpers.h"


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

static ffa_endpoint_id16_t spmc_id;
static ffa_endpoint_id16_t partition_id;

/* Partition Mailbox */
static uint8_t send_page[PAGE_SIZE] __aligned(PAGE_SIZE);
static uint8_t recv_page[PAGE_SIZE] __aligned(PAGE_SIZE);

/*
 * Declare a global mailbox for use within the TSP.
 * This will be initialized appropriately when the buffers
 * are mapped with the SPMC.
 */
static struct mailbox mailbox;

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
 * This enum is used to handle test cases driven from the FF-A Test Driver.
 ******************************************************************************/
/* Keep in Sync with FF-A Test Driver. */
enum message_t {
	/* Partition Only Messages. */
	FF_A_RELAY_MESSAGE = 0,

	/* Basic Functionality. */
	FF_A_ECHO_MESSAGE,
	FF_A_RELAY_MESSAGE_EL3,

	/* Memory Sharing. */
	FF_A_MEMORY_SHARE,
	FF_A_MEMORY_SHARE_FRAGMENTED,
	FF_A_MEMORY_LEND,
	FF_A_MEMORY_LEND_FRAGMENTED,

	FF_A_MEMORY_SHARE_MULTI_ENDPOINT,
	FF_A_MEMORY_LEND_MULTI_ENDPOINT,

	LAST,
	FF_A_RUN_ALL = 255,
	FF_A_OP_MAX = 256
};

/*******************************************************************************
 * Wrapper function to send a direct response.
 ******************************************************************************/
static tsp_args_t *ffa_msg_send_direct_resp(ffa_endpoint_id16_t sender,
					    ffa_endpoint_id16_t receiver,
					    uint32_t arg3,
					    uint32_t arg4,
					    uint32_t arg5,
					    uint32_t arg6,
					    uint32_t arg7)
{
	uint32_t src_dst_ids = (sender << FFA_DIRECT_MSG_SOURCE_SHIFT) |
			       (receiver << FFA_DIRECT_MSG_DESTINATION_SHIFT);

	return set_smc_args(FFA_MSG_SEND_DIRECT_RESP_SMC64, src_dst_ids,
			    0, arg3, arg4, arg5, arg6, arg7);
}

/*******************************************************************************
 * Wrapper function to send a direct request.
 ******************************************************************************/
static tsp_args_t ffa_msg_send_direct_req(ffa_endpoint_id16_t sender,
					  ffa_endpoint_id16_t receiver,
					  uint32_t arg3,
					  uint32_t arg4,
					  uint32_t arg5,
					  uint32_t arg6,
					  uint32_t arg7)
{
	uint32_t src_dst_ids = (sender << FFA_DIRECT_MSG_SOURCE_SHIFT) |
			       (receiver << FFA_DIRECT_MSG_DESTINATION_SHIFT);


	/* Send Direct Request. */
	return tsp_smc(FFA_MSG_SEND_DIRECT_REQ_SMC64, src_dst_ids,
			0, arg3, arg4, arg5, arg6, arg7);
}

/*******************************************************************************
 * Memory Management Helpers.
 ******************************************************************************/
static uint8_t mem_region_buffer[4096 * 2]  __aligned(PAGE_SIZE);
#define REGION_BUF_SIZE sizeof(mem_region_buffer)

static bool memory_retrieve(struct mailbox *mb,
			    struct ffa_mtd **retrieved,
			    uint64_t handle, ffa_endpoint_id16_t sender, ffa_endpoint_id16_t *receivers,
			    uint32_t receiver_count, ffa_mtd_flag32_t flags, uint32_t *frag_length,
			    uint32_t *total_length)
{
	tsp_args_t ret;
	uint32_t descriptor_size;
	struct ffa_mtd *memory_region = (struct ffa_mtd *)mb->tx_buffer;

	if (retrieved == NULL || mb == NULL) {
		ERROR("Invalid parameters!\n");
		return false;
	}

	/* Clear TX buffer. */
	memset(memory_region, 0, PAGE_SIZE);

	/* Clear local buffer. */
	memset(mem_region_buffer, 0, REGION_BUF_SIZE);

	descriptor_size = ffa_memory_retrieve_request_init(
	    memory_region, handle, sender, receivers, receiver_count, 0, flags,
	    FFA_MEM_PERM_RW | FFA_MEM_PERM_NX,
	    FFA_MEM_ATTR_NORMAL_MEMORY_CACHED_WB |
	    FFA_MEM_ATTR_INNER_SHAREABLE);

	ret = ffa_mem_retrieve_req(descriptor_size, descriptor_size);

	if (ffa_func_id(ret) == FFA_ERROR) {
		ERROR("Couldn't retrieve the memory page. Error: %x\n",
		      ffa_error_code(ret));
		return false;
	}

	/*
	 * Following total_size and fragment_size are useful to keep track
	 * of the state of transaction. When the sum of all fragment_size of all
	 * fragments is equal to total_size, the memory transaction has been
	 * completed.
	 */
	*total_length = ret._regs[1];
	*frag_length = ret._regs[2];

	/* Validate frag_length is less than total_length and mailbox size. */
	if (*frag_length == 0U || *total_length == 0U ||
			*frag_length > *total_length || *frag_length > mb->rxtx_page_count*PAGE_SIZE) {
		ERROR("Invalid parameters!\n");
		return false;
	}

	/* Copy response to local buffer. */
	memcpy(mem_region_buffer, mb->rx_buffer, *frag_length);

	if (ffa_rx_release()) {
		ERROR("Failed to release buffer!\n");
		return false;
	}

	*retrieved = (struct ffa_mtd *) mem_region_buffer;

	if ((*retrieved)->emad_count > MAX_MEM_SHARE_RECIPIENTS) {
		VERBOSE("SPMC memory sharing supports max of %u receivers!\n",
			MAX_MEM_SHARE_RECIPIENTS);
		return false;
	}

	/*
	 * We are sharing memory from the normal world therefore validate the NS
	 * bit was set by the SPMC.
	 */
	if (((*retrieved)->memory_region_attributes & FFA_MEM_ATTR_NS_BIT) == 0U) {
		ERROR("SPMC has not set the NS bit! 0x%x\n",
		      (*retrieved)->memory_region_attributes);
		return false;
	}

	VERBOSE("Memory Descriptor Retrieved!\n");

	return true;
}

/*******************************************************************************
 * Test Functions.
 ******************************************************************************/

/*******************************************************************************
 * Enable the TSP to forward the received message to another partition and ask
 * it to echo the value back in order to validate direct messages functionality.
 ******************************************************************************/
static int ffa_test_relay(uint64_t arg0,
			  uint64_t arg1,
			  uint64_t arg2,
			  uint64_t arg3,
			  uint64_t arg4,
			  uint64_t arg5,
			  uint64_t arg6,
			  uint64_t arg7)
{
	tsp_args_t ffa_forward_result;
	ffa_endpoint_id16_t receiver = arg5;

	ffa_forward_result = ffa_msg_send_direct_req(ffa_endpoint_source(arg1),
						     receiver,
						     FF_A_ECHO_MESSAGE, arg4,
						     0, 0, 0);
	return ffa_forward_result._regs[3];
}

/*******************************************************************************
 * This function handles memory management tests, currently share and lend.
 * This test supports the use of FRAG_RX to use memory descriptors that do not
 * fit in a single 4KB buffer.
 ******************************************************************************/
static int test_memory_send(ffa_endpoint_id16_t sender, uint64_t handle,
			    bool share, bool multi_endpoint)
{
	struct ffa_mtd *m;
	struct ffa_emad_v1_0 *receivers;
	struct ffa_comp_mrd *composite;
	int ret, status = 0;
	unsigned int mem_attrs;
	char *ptr;
	ffa_endpoint_id16_t source = sender;
	ffa_mtd_flag32_t flags = share ? FFA_FLAG_SHARE_MEMORY : FFA_FLAG_LEND_MEMORY;
	uint32_t total_length, recv_length = 0;

	/*
	 * In the case that we're testing multiple endpoints choose a partition
	 * ID that resides in the normal world so the SPMC won't detect it as
	 * invalid.
	 */
	uint32_t receiver_count = multi_endpoint ? 2 : 1;
	ffa_endpoint_id16_t test_receivers[2] = { partition_id, 0x10 };

	if (!memory_retrieve(&mailbox, &m, handle, source, test_receivers,
			     receiver_count, flags, &recv_length,
			     &total_length)) {
		return FFA_ERROR_INVALID_PARAMETER;
	}

	receivers = (struct ffa_emad_v1_0 *)
		    ((uint8_t *) m + m->emad_offset);
	while (total_length != recv_length) {
		tsp_args_t ffa_return;
		uint32_t frag_length;

		ffa_return = ffa_mem_frag_rx(handle, recv_length);

		if (ffa_return._regs[0] == FFA_ERROR) {
			WARN("TSP: failed to resume mem with handle %lx\n",
			     handle);
			return ffa_return._regs[2];
		}
		frag_length = ffa_return._regs[3];

		/* Validate frag_length is less than total_length and mailbox size. */
		if (frag_length > total_length || frag_length > mailbox.rxtx_page_count*PAGE_SIZE) {
			ERROR("Invalid parameters!\n");
			return false;
		}

		memcpy(&mem_region_buffer[recv_length], mailbox.rx_buffer,
		       frag_length);

		if (ffa_rx_release()) {
			ERROR("Failed to release buffer!\n");
			return FFA_ERROR_DENIED;
		}

		recv_length += frag_length;

		assert(recv_length <= total_length);
	}

	composite = ffa_memory_region_get_composite(m, 0);
	if (composite == NULL) {
		WARN("Failed to get composite descriptor!\n");
		return FFA_ERROR_INVALID_PARAMETER;
	}

	VERBOSE("Address: %p; page_count: %x %lx\n",
		(void *)composite->address_range_array[0].address,
		composite->address_range_array[0].page_count, PAGE_SIZE);

	/* This test is only concerned with RW permissions. */
	if (ffa_get_data_access_attr(
	    receivers[0].mapd.memory_access_permissions) != FFA_MEM_PERM_RW) {
		ERROR("Data permission in retrieve response %x does not match share/lend %x!\n",
		      ffa_get_data_access_attr(receivers[0].mapd.memory_access_permissions),
		      FFA_MEM_PERM_RW);
		return FFA_ERROR_INVALID_PARAMETER;
	}

	mem_attrs = MT_RW_DATA | MT_EXECUTE_NEVER;

	/* Only expecting to be sent memory from NWd so map accordingly. */
	mem_attrs |= MT_NS;

	for (uint32_t i = 0U; i < composite->address_range_count; i++) {
		size_t size = composite->address_range_array[i].page_count * PAGE_SIZE;

		ret = mmap_add_dynamic_region(
				(uint64_t)composite->address_range_array[i].address,
				(uint64_t)composite->address_range_array[i].address,
				size, mem_attrs);

		if (ret != 0) {
			ERROR("Failed [%d] mmap_add_dynamic_region %d (%lx) (%lx) (%x)!\n",
				i, ret,
				(uint64_t)composite->address_range_array[i].address,
				size, mem_attrs);

			/* Remove mappings created in this transaction. */
	        	for (i--; i >= 0U; i--) {
        	        	ret = mmap_remove_dynamic_region(
                	        	(uint64_t)composite->address_range_array[i].address,
	                        	composite->address_range_array[i].page_count * PAGE_SIZE);

		                if (ret != 0) {
        		                ERROR("Failed [%d] mmap_remove_dynamic_region!\n", i);
                		        panic();
	                	}
			}
			return FFA_ERROR_NO_MEMORY;
		}

		ptr = (char *) composite->address_range_array[i].address;

		/* Increment memory region for validation purposes. */
		++(*ptr);

		/*
		 * Read initial magic number from memory region for
		 * validation purposes.
		 */
		if (!i) {
			status = *ptr;
		}
	}

	for (uint32_t i = 0U; i < composite->address_range_count; i++) {
		ret = mmap_remove_dynamic_region(
			(uint64_t)composite->address_range_array[i].address,
			composite->address_range_array[i].page_count * PAGE_SIZE);

		if (ret != 0) {
			ERROR("Failed [%d] mmap_remove_dynamic_region!\n", i);
			return FFA_ERROR_NO_MEMORY;
		}
	}
	if (!memory_relinquish((struct ffa_mem_relinquish_descriptor *)mailbox.tx_buffer,
				m->handle, partition_id)) {
		ERROR("Failed to relinquish memory region!\n");
		return FFA_ERROR_INVALID_PARAMETER;
	}
	return status;
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

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
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

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
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

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
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

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
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

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
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

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
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
 * Handles partition messages. Exercised from the FF-A Test Driver.
 ******************************************************************************/
static tsp_args_t *handle_partition_message(uint64_t arg0,
					    uint64_t arg1,
					    uint64_t arg2,
					    uint64_t arg3,
					    uint64_t arg4,
					    uint64_t arg5,
					    uint64_t arg6,
					    uint64_t arg7)
{
	uint16_t sender = ffa_endpoint_source(arg1);
	uint16_t receiver = ffa_endpoint_destination(arg1);
	int status = -1;


	const bool share = true;
	const bool multi_endpoint = true;

	switch (arg3) {
	case FF_A_MEMORY_SHARE:
		INFO("TSP Tests: Memory Share Request--\n");
		status = test_memory_send(sender, arg4, share, !multi_endpoint);
		break;

	case FF_A_MEMORY_LEND:
		INFO("TSP Tests: Memory Lend Request--\n");
		status = test_memory_send(sender, arg4, !share, !multi_endpoint);
		break;

	case FF_A_MEMORY_SHARE_MULTI_ENDPOINT:
		INFO("TSP Tests: Multi Endpoint Memory Share Request--\n");
		status = test_memory_send(sender, arg4, share, multi_endpoint);
		break;

	case FF_A_MEMORY_LEND_MULTI_ENDPOINT:
		INFO("TSP Tests: Multi Endpoint Memory Lend Request--\n");
		status = test_memory_send(sender, arg4, !share, multi_endpoint);
		break;
	case FF_A_RELAY_MESSAGE:
		INFO("TSP Tests: Relaying message--\n");
		status = ffa_test_relay(arg0, arg1, arg2, arg3, arg4,
					arg5, arg6, arg7);
		break;

	case FF_A_ECHO_MESSAGE:
		INFO("TSP Tests: echo message--\n");
		status = arg4;
		break;

	default:
		INFO("TSP Tests: Unknown request ID %d--\n", (int) arg3);
	}

	/* Swap the sender and receiver in the response. */
	return ffa_msg_send_direct_resp(receiver, sender, status, 0, 0, 0, 0);
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

	case FFA_MSG_SEND_DIRECT_REQ_SMC64:
	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
		/* Check if a framework message, handle accordingly. */
		if ((arg2 & FFA_FWK_MSG_BIT)) {
			return handle_framework_message(smc_fid, arg1, arg2, arg3,
							arg4, arg5, arg6, arg7);
		}
		return handle_partition_message(smc_fid, arg1, arg2, arg3,
							arg4, arg5, arg6, arg7);
	default:
		return set_smc_args(FFA_MSG_SEND_DIRECT_RESP_SMC32, 1, 2, 3, 4,
					0, 0, 0);
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

	partition_id =  smc_args._regs[2];

	/* Get the SPMC ID */
	smc_args = tsp_smc(FFA_SPM_ID_GET, 0, 0, 0, 0, 0, 0, 0);
	if (smc_args._regs[TSP_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not get SPMC ID (0x%lx) on core%d\n",
				smc_args._regs[2], linear_id);
		panic();
	}

	spmc_id = smc_args._regs[2];

	/* Call RXTX_MAP to map a 4k RX and TX buffer. */
	if (ffa_rxtx_map((uintptr_t) send_page,
			 (uintptr_t) recv_page, 1)) {
		ERROR("TSP could not map it's RX/TX Buffers\n");
		panic();
	}

	mailbox.tx_buffer = send_page;
	mailbox.rx_buffer = recv_page;
	mailbox.rxtx_page_count = 1;

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
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
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
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
