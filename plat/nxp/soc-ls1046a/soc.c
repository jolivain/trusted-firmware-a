/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <arch.h>
#include <bl31/interrupt_mgmt.h>
#include <caam.h>
#include <cassert.h>
#include <cci.h>
#include <common/debug.h>
#include <csu.h>
#include <dcfg.h>
#ifdef I2C_INIT
#include <i2c.h>
#endif
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <ls_interconnect.h>
#ifdef POLICY_FUSE_PROVISION
#include <nxp_gpio.h>
#endif
#if TRUSTED_BOARD_BOOT
#include <nxp_smmu.h>
#endif
#include <nxp_timer.h>
#include <plat_console.h>
#include <plat_gic.h>
#include <plat_tzc400.h>
#include <scfg.h>
#if defined(NXP_SFP_ENABLED)
#include <sfp.h>
#endif

#include <errata.h>
#include <ls_interrupt_mgmt.h>
#ifdef IMAGE_BL31
#include <ns_access.h>
#endif
#include "plat_common.h"
#ifdef NXP_NV_SW_MAINT_LAST_EXEC_DATA
#include <plat_nv_storage.h>
#endif
#include "platform_def.h"
#include "soc.h"

static dcfg_init_info_t dcfg_init_data = {
			.g_nxp_dcfg_addr = NXP_DCFG_ADDR,
			.nxp_sysclk_freq = NXP_SYSCLK_FREQ,
			.nxp_ddrclk_freq = NXP_DDRCLK_FREQ,
			.nxp_plat_clk_divider = NXP_PLATFORM_CLK_DIVIDER,
		};


/* Function to return the SoC SYS CLK
 */
unsigned int get_sys_clk(void)
{
	return NXP_SYSCLK_FREQ;
}

/*
 * Function returns the base counter frequency
 * after reading the first entry at CNTFID0 (0x20 offset).
 *
 * Function is used by:
 *   1. ARM common code for PSCI management.
 *   2. ARM Generic Timer init.
 *
 */
unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;
	/*
	 * Below register specifies the base frequency of the system counter.
	 * As per NXP Board Manuals:
	 * The system counter always works with SYS_REF_CLK/4 frequency clock.
	 *
	 * Not reading the frequency from Frequency modes table.
	 *
	 * Note: The value for ls1046ardb board at this offset is not RW and
	 *       have the fixed value of 100000400 Hz.
	 */
	counter_base_frequency = get_sys_clk()/4;

	return counter_base_frequency;
}

#ifdef IMAGE_BL2

static struct soc_type soc_list[] =  {
	SOC_ENTRY(LS1046A, LS1046A, 1, 4),
	SOC_ENTRY(LS1026A, LS1026A, 1, 2),
};

#ifdef POLICY_FUSE_PROVISION
static gpio_init_info_t gpio_init_data = {
	.gpio1_base_addr = NXP_GPIO1_ADDR,
	.gpio2_base_addr = NXP_GPIO2_ADDR,
	.gpio3_base_addr = NXP_GPIO3_ADDR,
	.gpio4_base_addr = NXP_GPIO4_ADDR,
};
#endif

/*
 * Function to set the base counter frequency at
 * the first entry of the Frequency Mode Table,
 * at CNTFID0 (0x20 offset).
 *
 * Set the value of the pirmary core register cntfrq_el0.
 */
static void set_base_freq_CNTFID0(void)
{
	/*
	 * Below register specifies the base frequency of the system counter.
	 * As per NXP Board Manuals:
	 * The system counter always works with SYS_REF_CLK/4 frequency clock.
	 *
	 */
	unsigned int counter_base_frequency = get_sys_clk()/4;

	/* Setting the frequency in the Frequency modes table.
	 *
	 * Note: The value for ls1046ardb board at this offset
	 *       is not RW as stated. This offset have the
	 *       fixed value of 100000400 Hz.
	 *
	 * The below code line has no effect.
	 * Keeping it for other platforms where it has effect.
	 */
	mmio_write_32(NXP_TIMER_ADDR + CNTFID_OFF, counter_base_frequency);

	write_cntfrq_el0(counter_base_frequency);
}


/*
 * This function returns the number of clusters in the SoC
 */
static unsigned int get_num_cluster(void)
{
	const soc_info_t *soc_info = get_soc_info();
	uint32_t num_clusters = NUMBER_OF_CLUSTERS;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(soc_list); i++) {
		if (soc_list[i].personality == soc_info->personality) {
			num_clusters = soc_list[i].num_clusters;
			break;
		}
	}

	VERBOSE("NUM of cluster = 0x%x\n", num_clusters);

	return num_clusters;
}

void soc_preload_setup(void)
{

}

/*******************************************************************************
 * This function implements soc specific erratas
 * This is called before DDR is initialized or MMU is enabled
 ******************************************************************************/
void soc_early_init(void)
{
	dram_regions_info_t *dram_regions_info = get_dram_regions_info();

	dcfg_init(&dcfg_init_data);
#ifdef POLICY_FUSE_PROVISION
	gpio_init(&gpio_init_data);
	sec_init(NXP_CAAM_ADDR);
#endif
#if LOG_LEVEL > 0
	/* Initialize the console to provide early debug support */

	plat_console_init(NXP_CONSOLE_ADDR,
				NXP_UART_CLK_DIVIDER, NXP_CONSOLE_BAUDRATE);
#endif
	set_base_freq_CNTFID0();

	/* Enable snooping on SEC read and write transactions */
	scfg_setbits32((void *)(NXP_SCFG_ADDR + SCFG_SNPCNFGCR_OFFSET),
			SCFG_SNPCNFGCR_SECRDSNP | SCFG_SNPCNFGCR_SECWRSNP);

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_ls_interconnect_enter_coherency(get_num_cluster());


#if TRUSTED_BOARD_BOOT
	uint32_t mode;

	sfp_init(NXP_SFP_ADDR);
	/* For secure boot disable SMMU.
	 * Later when platform security policy comes in picture,
	 * this might get modified based on the policy
	 */
	if (check_boot_mode_secure(&mode) == true) {
		bypass_smmu(NXP_SMMU_ADDR);
	}

	/* For Mbedtls currently crypto is not supported via CAAM
	 * enable it when that support is there. In tbbr.mk
	 * the CAAM_INTEG is set as 0.
	 */

#ifndef MBEDTLS_X509
	/* Initialize the crypto accelerator if enabled */
	if (is_sec_enabled() == false) {
		INFO("SEC is disabled.\n");
	} else {
		sec_init(NXP_CAAM_ADDR);
	}
#endif
#elif defined(POLICY_FUSE_PROVISION)
	gpio_init(&gpio_init_data);
	sfp_init(NXP_SFP_ADDR);
	sec_init(NXP_CAAM_ADDR);
#endif

	/* Add Erratums here */
	erratum_a008850_early();
	erratum_a010539();

	/*
	 * Initialize system level generic timer for Layerscape Socs.
	 */
	delay_timer_init(NXP_TIMER_ADDR);

#ifdef DDR_INIT
	i2c_init(NXP_I2C_ADDR);
	dram_regions_info->total_dram_size = init_ddr();
#endif
}

void soc_bl2_prepare_exit(void)
{
#if defined(NXP_SFP_ENABLED) && defined(DISABLE_FUSE_WRITE)
	set_sfp_wr_disable();
#endif
}

/*****************************************************************************
 * This function returns the boot device based on RCW_SRC
 ****************************************************************************/
enum boot_device get_boot_dev(void)
{
	enum boot_device src = BOOT_DEVICE_NONE;
	uint32_t porsr1;
	uint32_t rcw_src, val;

	porsr1 = read_reg_porsr1();

	rcw_src = (porsr1 & PORSR1_RCW_MASK) >> PORSR1_RCW_SHIFT;

	val = rcw_src & RCW_SRC_NAND_MASK;

	if (val == RCW_SRC_NAND_VAL) {
		val = rcw_src & NAND_RESERVED_MASK;
		if ((val != NAND_RESERVED_1) && (val != NAND_RESERVED_2)) {
			src = BOOT_DEVICE_IFC_NAND;
			INFO("RCW BOOT SRC is IFC NAND\n");
		}
	} else {
		/* RCW SRC NOR */
		val = rcw_src & RCW_SRC_NOR_MASK;
		if (val == NOR_8B_VAL || val == NOR_16B_VAL) {
			src = BOOT_DEVICE_IFC_NOR;
			INFO("RCW BOOT SRC is IFC NOR\n");
		} else {
			switch (rcw_src) {
			case QSPI_VAL1:
			case QSPI_VAL2:
				src = BOOT_DEVICE_QSPI;
				INFO("RCW BOOT SRC is QSPI\n");
				break;
			case SD_VAL:
				src = BOOT_DEVICE_EMMC;
				INFO("RCW BOOT SRC is SD/EMMC\n");
				break;
			default:
				src = BOOT_DEVICE_NONE;
			}
		}
	}

	return src;
}

void soc_mem_access(void)
{
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
	struct tzc400_reg tzc400_reg_list[MAX_NUM_TZC_REGION];
	int dram_idx, index = 0;

	for (dram_idx = 0; dram_idx < info_dram_regions->num_dram_regions;
			dram_idx++) {
		if (info_dram_regions->region[dram_idx].size == 0) {
			ERROR("DDR init failure, or");
			ERROR("DRAM regions not populated correctly.\n");
			break;
		}

		index = populate_tzc400_reg_list(tzc400_reg_list,
				dram_idx, index,
				info_dram_regions->region[dram_idx].addr,
				info_dram_regions->region[dram_idx].size,
				NXP_SECURE_DRAM_SIZE, NXP_SP_SHRD_DRAM_SIZE);
	}

	mem_access_setup(NXP_TZC_ADDR, index, tzc400_reg_list);
}

#else
const unsigned char _power_domain_tree_desc[] = {1, 1, 4};

CASSERT(NUMBER_OF_CLUSTERS && NUMBER_OF_CLUSTERS <= 256,
		assert_invalid_ls1046_cluster_count);

/*
 * This function returns the SoC topology
 */

const unsigned char *plat_get_power_domain_tree_desc(void)
{

	return _power_domain_tree_desc;
}

/*
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 */
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr)
{
	return CORES_PER_CLUSTER;
}

void soc_early_platform_setup2(void)
{
	dcfg_init(&dcfg_init_data);
	/*
	 * Initialize system level generic timer for Socs
	 */
	delay_timer_init(NXP_TIMER_ADDR);

#if LOG_LEVEL > 0
	/* Initialize the console to provide early debug support */
	plat_console_init(NXP_CONSOLE_ADDR,
				NXP_UART_CLK_DIVIDER, NXP_CONSOLE_BAUDRATE);
#endif
}

void soc_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	static uint32_t target_mask_array[PLATFORM_CORE_COUNT];
	/* On a GICv2 system, the Group 1 secure interrupts are treated
	 * as Group 0 interrupts.
	 */
	static interrupt_prop_t ls_interrupt_props[] = {
		PLAT_LS_G1S_IRQ_PROPS(GICV2_INTR_GROUP0),
		PLAT_LS_G0_IRQ_PROPS(GICV2_INTR_GROUP0)
	};

	plat_ls_gic_driver_init(
#if (TEST_BL31)
		/* Defect in simulator - GIC base addresses (4Kb aligned)
		 */
				NXP_GICD_4K_ADDR,
				NXP_GICC_4K_ADDR,
#else
				NXP_GICD_64K_ADDR,
				NXP_GICC_64K_ADDR,
#endif
				PLATFORM_CORE_COUNT,
				ls_interrupt_props,
				ARRAY_SIZE(ls_interrupt_props),
				target_mask_array);

	plat_ls_gic_init();
	enable_init_timer();
}

/*******************************************************************************
 * This function initializes the soc from the BL31 module
 ******************************************************************************/
void soc_init(void)
{
	 /* low-level init of the soc */
	soc_init_start();
	soc_init_percpu();
	_init_global_data();
	_initialize_psci();

	/*
	 * Initialize the interconnect during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this but we can't
	 * assume so. No harm in executing this code twice.
	 */
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr()));

	/* Init CSU to enable non-secure access to peripherals */
	enable_layerscape_ns_access(ns_dev, ARRAY_SIZE(ns_dev), NXP_CSU_ADDR);

	 /* make sure any parallel init tasks are finished */
	soc_init_finish();

	/* Initialize the crypto accelerator if enabled */
	if (is_sec_enabled() == false) {
		INFO("SEC is disabled.\n");
	} else {
		sec_init(NXP_CAAM_ADDR);
	}
}

void soc_runtime_setup(void)
{

}
#endif
