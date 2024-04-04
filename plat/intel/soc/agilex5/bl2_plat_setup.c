/*
 * Copyright (c) 2019-2021, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/cadence/cdns_sdmmc.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/synopsys/dw_mmc.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "agilex5_clock_manager.h"
#include "agilex5_memory_controller.h"
#include "agilex5_mmc.h"
#include "agilex5_pinmux.h"
#include "agilex5_power_manager.h"
#include "agilex5_system_manager.h"
#include "ccu/ncore_ccu.h"
#include "combophy/combophy.h"
#include "nand/nand.h"
#include "qspi/cadence_qspi.h"
#include "sdmmc/sdmmc.h"
#include "socfpga_emac.h"
#include "socfpga_f2sdram_manager.h"
#include "socfpga_handoff.h"
#include "socfpga_mailbox.h"
#include "socfpga_private.h"
#include "socfpga_reset_manager.h"
#include "socfpga_ros.h"
#include "socfpga_vab.h"
#include "wdt/watchdog.h"

/* Declare mmc_info */
static struct mmc_device_info mmc_info;

/* Declare cadence idmac descriptor */
extern struct cdns_idmac_desc cdns_desc[8] __aligned(32);

const mmap_region_t agilex_plat_mmap[] = {
	MAP_REGION_FLAT(DRAM_BASE, DRAM_SIZE,
		MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(PSS_BASE, PSS_SIZE,
		MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(MPFE_BASE, MPFE_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(OCRAM_BASE, OCRAM_SIZE,
		MT_NON_CACHEABLE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CCU_BASE, CCU_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MEM64_BASE, MEM64_SIZE,
		MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(GIC_BASE, GIC_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	{0},
};

boot_source_type boot_source = BOOT_SOURCE;

void bl2_el3_early_platform_setup(u_register_t x0 __unused,
				  u_register_t x1 __unused,
				  u_register_t x2 __unused,
				  u_register_t x3 __unused)
{
	static console_t console;
	handoff reverse_handoff_ptr = { 0 };

	/* Bring all the required peripherals out of reset */
	deassert_peripheral_reset();

	/* Enable nonsecure access for peripherals and other misc components */
	enable_nonsecure_access();

	/*
	 * Initialize the UART console early in BL2 EL3 boot flow to get
	 * the error/notice messages wherever required.
	 */
	console_16550_register(PLAT_INTEL_UART_BASE, PLAT_UART_CLOCK,
			PLAT_BAUDRATE, &console);

	/* Get the hand-off data */
	if (socfpga_get_handoff(&reverse_handoff_ptr)) {
		ERROR("BL2: Failed to get the correct hand-off data\n");
		panic();
	}

	/* Configure the pinmux */
	config_pinmux(&reverse_handoff_ptr);

	/* Configure the clock manager */
	if (config_clkmgr_handoff(&reverse_handoff_ptr)) {
		ERROR("BL2: Failed to initialize the clock manager\n");
		panic();
	}

	/* Generic delay timer init */
	generic_delay_timer_init();

	/* Perform a handshake with certain peripherals before issuing a reset */
	config_hps_hs_before_warm_reset();

	/* TODO: watchdog init */
	//watchdog_init(clkmgr_get_rate(CLKMGR_WDT_CLK_ID));

	socfpga_delay_timer_init();

	/* Configure power manager PSS SRAM power gate */
	config_pwrmgr_handoff(&reverse_handoff_ptr);

	/*
	 * TODO update list for BL2 EL3 init:
	 * 1. DDR and IOSSM driver init
	 * 2. Update CCU driver to match SM configuration, make it dynamic based on hand-off
	 * 3. Update Combo PHY init and remove hard coding
	 */

	init_ncore_ccu();

	socfpga_emac_init();

	mailbox_init();

	if (combo_phy_init(&reverse_handoff_ptr) != 0) {
		ERROR("Combo Phy initialization failed\n");
	}

	/* Store magic number */
	// TODO: Temp workaround to ungate testing
	// mmio_write_32(L2_RESET_DONE_REG, PLAT_L2_RESET_REQ);

	if (!intel_mailbox_is_fpga_not_ready()) {
		socfpga_bridges_enable(SOC2FPGA_MASK | LWHPS2FPGA_MASK |
					FPGA2SOC_MASK | F2SDRAM0_MASK);
	}
}

void bl2_el3_plat_arch_setup(void)
{
	handoff reverse_handoff_ptr;
	unsigned long offset = 0;

	struct cdns_sdmmc_params params = EMMC_INIT_PARAMS((uintptr_t) &cdns_desc,
							   clkmgr_get_rate(CLKMGR_SDMMC_CLK_ID));

	mmc_info.mmc_dev_type = MMC_DEVICE_TYPE;
	mmc_info.ocr_voltage = OCR_3_3_3_4 | OCR_3_2_3_3;

	/* Request ownership and direct access to QSPI */
	mailbox_hps_qspi_enable();

	switch (boot_source) {
	case BOOT_SOURCE_SDMMC:
		NOTICE("SDMMC boot\n");
		sdmmc_init(&reverse_handoff_ptr, &params, &mmc_info);
		socfpga_io_setup(boot_source, PLAT_SDMMC_DATA_BASE);
		break;

	case BOOT_SOURCE_QSPI:
		NOTICE("QSPI boot\n");
		cad_qspi_init(0, QSPI_CONFIG_CPHA, QSPI_CONFIG_CPOL,
			QSPI_CONFIG_CSDA, QSPI_CONFIG_CSDADS,
			QSPI_CONFIG_CSEOT, QSPI_CONFIG_CSSOT, 0);
		if (ros_qspi_get_ssbl_offset(&offset) != ROS_RET_OK) {
			offset = PLAT_QSPI_DATA_BASE;
		}
		socfpga_io_setup(boot_source, offset);
		break;

	case BOOT_SOURCE_NAND:
		NOTICE("NAND boot\n");
		nand_init(&reverse_handoff_ptr);
		socfpga_io_setup(boot_source, PLAT_NAND_DATA_BASE);
		break;

	default:
		ERROR("Unsupported boot source\n");
		panic();
		break;
	}
}

uint32_t get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	assert(bl_mem_params);

#if SOCFPGA_SECURE_VAB_AUTH
	/*
	 * VAB Authentication start here.
	 * If failed to authenticate, shall not proceed to process BL31 and hang.
	 */
	int ret = 0;

	ret = socfpga_vab_init(image_id);
	if(ret < 0) {
		ERROR("SOCFPGA VAB Authentication failed\n");
		while (1)
		wfi();
	}
#endif

	switch (image_id) {
	case BL33_IMAGE_ID:
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = get_spsr_for_bl33_entry();
		break;
	default:
		break;
	}

	return 0;
}

/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl2_platform_setup(void)
{
}