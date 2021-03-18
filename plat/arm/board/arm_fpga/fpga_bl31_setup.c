/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/extensions/spe.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <libfdt.h>

#include "fpga_private.h"
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#define MAP_BL31_TOTAL		MAP_REGION_FLAT(			\
					BL31_START,			\
					BL31_END - BL31_START,		\
					MT_MEMORY | MT_RW | MT_SECURE)

static entry_point_info_t bl33_image_ep_info;
volatile uint32_t secondary_core_spinlock;

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return 0ULL;
#endif
}

uint32_t fpga_get_spsr_for_bl33_entry(void)
{
	return SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Add this core to the VALID mpids list */
	fpga_valid_mpids[plat_my_core_pos()] = VALID_MPID;

	/*
	 * Notify the secondary CPUs that the C runtime is ready
	 * so they can announce themselves.
	 */
	secondary_core_spinlock = C_RUNTIME_READY_KEY;
	dsbish();
	sev();

	fpga_console_init();

	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = fpga_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	/* Set x0-x3 for the primary CPU as expected by the kernel */
	bl33_image_ep_info.args.arg0 = (u_register_t)FPGA_PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
}

void __init fpga_bl31_plat_arch_setup(void)
{
	uintptr_t gicd_base, gicr_base, uart_base, fdt_uart_base;
	size_t gicd_size, gicr_size, uart_size, fdt_uart_size;

	const void *fdt = (void *)(uintptr_t)FPGA_PRELOADED_DTB_BASE;
	int node, ret;

	const mmap_region_t bl_regions[] = {
		MAP_BL31_TOTAL,
		ARM_MAP_BL_RO,
		{0}
	};

	/*
	 * Read GICD and GICR base addresses and sizes from the DT.
	 */
	node = fdt_node_offset_by_compatible(fdt, 0, "arm,gic-v3");
	if (node < 0) {
		ERROR("No \"arm,gic-v3\" compatible node found in DT, no GIC support.\n");
		panic();
	}

	/* TODO: Assuming only empty "ranges;" properties up the bus path. */
	ret = fdt_get_reg_props_by_index(fdt, node, 0,
				 &gicd_base, &gicd_size);
	if (ret < 0) {
		ERROR("Could not read GIC distributor address from DT.\n");
		panic();
	}

	ret = fdt_get_reg_props_by_index(fdt, node, 1,
				 &gicr_base, &gicr_size);
	if (ret < 0) {
		ERROR("Could not read GIC redistributor address from DT.\n");
		panic();
	}

	/*
	 * Read UART base address and size from the DT, falling back to
	 * PLAT_FPGA_CRASH_UART_BASE as a default if this fails.
	 */
	uart_base = PLAT_FPGA_CRASH_UART_BASE;
	uart_size = PAGE_SIZE;

	node = fdt_get_stdout_node_offset(fdt);
	if (node >= 0) {
		ret = fdt_get_reg_props_by_index(fdt, node, 0,
						 &fdt_uart_base,
						 &fdt_uart_size);
		if (ret == 0) {
			uart_base = fdt_uart_base;
			uart_size = fdt_uart_size;
		} else {
			WARN("Could not read UART address from DT, using hardcoded default.\n");
		}
	} else {
		WARN("Could not read UART address from DT, using hardcoded default.\n");
	}

	/*
	 * Add the memory regions read from the DT.
	 *
	 * FIXME: below code is a bit "ad-hoc", as mmap_add_region calls should
	 * be done from setup_pable_tables() to make everything more
	 * consistent. However it does not seem to be a straight forward
	 * way of doing that (setup_page_tabes() receives consts and bl_regions
	 * is declared at the beginning of the fucntion to cope with MISRA).
	 * So just make the calls here and refactor the code in the future if
	 * it becomes possible.
	 */
	mmap_add_region((unsigned long long)uart_base, uart_base,
			uart_size, (MT_DEVICE | MT_RW | MT_SECURE));
	mmap_add_region((unsigned long long)gicd_base, gicd_base,
			gicd_size, (MT_DEVICE | MT_RW | MT_SECURE));
	/* Note: GICR size is multiplied by the core count */
	mmap_add_region((unsigned long long)gicr_base, gicr_base,
			gicr_size * PLATFORM_CORE_COUNT,
			(MT_DEVICE | MT_RW | MT_SECURE));

	setup_page_tables(bl_regions, plat_arm_get_mmap());

	enable_mmu_el3(0);
}

void __init bl31_plat_arch_setup(void)
{
	fpga_bl31_plat_arch_setup();
}

void bl31_platform_setup(void)
{
	/* Write frequency to CNTCRL and initialize timer */
	generic_delay_timer_init();

	/*
	 * Before doing anything else, wait for some time to ensure that
	 * the secondary CPUs have populated the fpga_valid_mpids array.
	 * As the number of secondary cores is unknown and can even be 0,
	 * it is not possible to rely on any signal from them, so use a
	 * delay instead.
	 */
	mdelay(5);

	/*
	 * On the event of a cold reset issued by, for instance, a reset pin
	 * assertion, we cannot guarantee memory to be initialized to zero.
	 * In such scenario, if the secondary cores reached
	 * plat_secondary_cold_boot_setup before the primary one initialized
	 * .BSS, we could end up having a race condition if the spinlock
	 * was not cleared before.
	 *
	 * Similarly, if there were a reset before the spinlock had been
	 * cleared, the secondary cores would find the lock opened before
	 * .BSS is cleared, causing another race condition.
	 *
	 * So clean the spinlock as soon as we think it is safe to reduce the
	 * chances of any race condition on a reset.
	 */
	secondary_core_spinlock = 0UL;

	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_fpga_gic_init();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;
	next_image_info = &bl33_image_ep_info;

	/* Only expecting BL33: the kernel will run in EL2NS */
	assert(type == NON_SECURE);

	/* None of the images can have 0x0 as the entrypoint */
	if (next_image_info->pc) {
		return next_image_info;
	} else {
		return NULL;
	}
}

unsigned int plat_get_syscnt_freq2(void)
{
	const void *fdt = (void *)(uintptr_t)FPGA_PRELOADED_DTB_BASE;
	int node;

	node = fdt_node_offset_by_compatible(fdt, 0, "arm,armv8-timer");
	if (node < 0) {
		return FPGA_DEFAULT_TIMER_FREQUENCY;
	}

	return fdt_read_uint32_default(fdt, node, "clock-frequency",
				       FPGA_DEFAULT_TIMER_FREQUENCY);
}

static void fpga_prepare_dtb(void)
{
	void *fdt = (void *)(uintptr_t)FPGA_PRELOADED_DTB_BASE;
	const char *cmdline = (void *)(uintptr_t)FPGA_PRELOADED_CMD_LINE;
	int err;

	err = fdt_open_into(fdt, fdt, FPGA_MAX_DTB_SIZE);
	if (err < 0) {
		ERROR("cannot open devicetree at %p: %d\n", fdt, err);
		panic();
	}

	/* Check for the command line signature. */
	if (!strncmp(cmdline, "CMD:", 4)) {
		int chosen;

		INFO("using command line at 0x%x\n", FPGA_PRELOADED_CMD_LINE);

		chosen = fdt_add_subnode(fdt, 0, "chosen");
		if (chosen == -FDT_ERR_EXISTS) {
			chosen = fdt_path_offset(fdt, "/chosen");
		}
		if (chosen < 0) {
			ERROR("cannot find /chosen node: %d\n", chosen);
		} else {
			const char *eol;
			char nul = 0;
			int slen;

			/*
			 * There is most likely an EOL at the end of the
			 * command line, make sure we terminate the line there.
			 * We can't replace the EOL with a NUL byte in the
			 * source, as this is in read-only memory. So we first
			 * create the property without any termination, then
			 * append a single NUL byte.
			 */
			eol = strchr(cmdline, '\n');
			if (!eol) {
				eol = strchr(cmdline, 0);
			}
			/* Skip the signature and omit the EOL/NUL byte. */
			slen = eol - (cmdline + 4);

			/*
			 * Let's limit the size of the property, just in case
			 * we find the signature by accident. The Linux kernel
			 * limits to 4096 characters at most (in fact 2048 for
			 * arm64), so that sounds like a reasonable number.
			 */
			if (slen > 4095) {
				slen = 4095;
			}
			err = fdt_setprop(fdt, chosen, "bootargs",
					  cmdline + 4, slen);
			if (!err) {
				err = fdt_appendprop(fdt, chosen, "bootargs",
						     &nul, 1);
			}
			if (err) {
				ERROR("Could not set command line: %d\n", err);
			}
		}
	}

	if (err < 0) {
		ERROR("Error %d extending Device Tree\n", err);
		panic();
	}

	err = fdt_add_cpus_node(fdt, FPGA_MAX_PE_PER_CPU,
				FPGA_MAX_CPUS_PER_CLUSTER,
				FPGA_MAX_CLUSTER_COUNT);

	if (err == -EEXIST) {
		WARN("Not overwriting already existing /cpus node in DTB\n");
	} else {
		if (err < 0) {
			ERROR("Error %d creating the /cpus DT node\n", err);
			panic();
		} else {
			unsigned int nr_cores = fpga_get_nr_gic_cores();

			INFO("Adjusting GICR DT region to cover %u cores\n",
			      nr_cores);
			err = fdt_adjust_gic_redist(fdt, nr_cores,
						    1U << GICR_PCPUBASE_SHIFT);
			if (err < 0) {
				ERROR("Error %d fixing up GIC DT node\n", err);
			}
		}
	}

	/* Check whether we support the SPE PMU. Remove the DT node if not. */
	if (!spe_supported()) {
		int node = fdt_node_offset_by_compatible(fdt, 0,
				     "arm,statistical-profiling-extension-v1");

		if (node >= 0) {
			fdt_del_node(fdt, node);
		}
	}

	err = fdt_pack(fdt);
	if (err < 0) {
		ERROR("Failed to pack Device Tree at %p: error %d\n", fdt, err);
	}

	clean_dcache_range((uintptr_t)fdt, fdt_blob_size(fdt));
}

void bl31_plat_runtime_setup(void)
{
	fpga_prepare_dtb();
}

void bl31_plat_enable_mmu(uint32_t flags)
{
	/* TODO: determine if MMU needs to be enabled */
}
