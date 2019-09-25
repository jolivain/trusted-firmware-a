/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/interrupt_props.h>
#include <drivers/console.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <cortex_a57.h>
#include <common/debug.h>
#include <denver.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>
#include <bl31/interrupt_mgmt.h>
#include <mce.h>
#include <plat/common/platform.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

/*******************************************************************************
 * The Tegra power domain tree has a single system level power domain i.e. a
 * single root node. The first entry in the power domain descriptor specifies
 * the number of power domains at the highest power level.
 *******************************************************************************
 */
const unsigned char tegra_power_domain_tree_desc[] = {
	/* No of root nodes */
	1,
	/* No of clusters */
	PLATFORM_CLUSTER_COUNT,
	/* No of CPU cores - cluster0 */
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	/* No of CPU cores - cluster1 */
	PLATFORM_MAX_CPUS_PER_CLUSTER
};

/*******************************************************************************
 * This function returns the Tegra default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return tegra_power_domain_tree_desc;
}

/*
 * Table of regions to map using the MMU.
 */
static const mmap_region_t tegra_mmap[] = {
	MAP_REGION_FLAT(TEGRA_MISC_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_TSA_BASE, 0x20000, /* 128KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MC_STREAMID_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MC_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_UARTA_BASE, 0x20000, /* 128KB - UART A, B*/
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_UARTC_BASE, 0x20000, /* 128KB - UART C, G */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_UARTD_BASE, 0x30000, /* 192KB - UART D, E, F */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_FUSE_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_GICD_BASE, 0x20000, /* 128KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SE0_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_PKA1_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_RNG1_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_CAR_RESET_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_PMC_BASE, 0x40000, /* 256KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SCRATCH_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MMCRAB_BASE, 0x60000, /* 384KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SMMU0_BASE, 0x1000000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SMMU1_BASE, 0x1000000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SMMU2_BASE, 0x1000000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_XUSB_PADCTL_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};

/*******************************************************************************
 * Set up the pagetables as per the platform memory map & initialize the MMU
 ******************************************************************************/
const mmap_region_t *plat_get_mmio_map(void)
{
	/* MMIO space */
	return tegra_mmap;
}

/*******************************************************************************
 * Handler to get the System Counter Frequency
 ******************************************************************************/
unsigned int plat_get_syscnt_freq2(void)
{
	return 31250000;
}

/*******************************************************************************
 * Maximum supported UART controllers
 ******************************************************************************/
#define TEGRA186_MAX_UART_PORTS		7

/*******************************************************************************
 * This variable holds the UART port base addresses
 ******************************************************************************/
static uint32_t tegra186_uart_addresses[TEGRA186_MAX_UART_PORTS + 1] = {
	0,	/* undefined - treated as an error case */
	TEGRA_UARTA_BASE,
	TEGRA_UARTB_BASE,
	TEGRA_UARTC_BASE,
	TEGRA_UARTD_BASE,
	TEGRA_UARTE_BASE,
	TEGRA_UARTF_BASE,
	TEGRA_UARTG_BASE,
};

/*******************************************************************************
 * Retrieve the UART controller base to be used as the console
 ******************************************************************************/
uint32_t plat_get_console_from_id(int id)
{
	if (id > TEGRA186_MAX_UART_PORTS)
		return 0;

	return tegra186_uart_addresses[id];
}

/*******************************************************************************
 * Handler for early platform setup
 ******************************************************************************/
void plat_early_platform_setup(void)
{

	/* sanity check MCE firmware compatibility */
	mce_verify_firmware_version();

	/* Program XUSB STREAMIDs
	 * Xavier XUSB has support for XUSB virtualization. It will have one
	 * physical function (PF) and four Virtual function (VF)
	 *
	 * There were below two SIDs for XUSB until T186.
	 * 1) #define TEGRA_SID_XUSB_HOST    0x1bU
	 * 2) #define TEGRA_SID_XUSB_DEV    0x1cU
	 *
	 * We have below four new SIDs added for VF(s)
	 * 3) #define TEGRA_SID_XUSB_VF0    0x5dU
	 * 4) #define TEGRA_SID_XUSB_VF1    0x5eU
	 * 5) #define TEGRA_SID_XUSB_VF2    0x5fU
	 * 6) #define TEGRA_SID_XUSB_VF3    0x60U
	 *
	 * When virtualization is enabled then we have to disable SID override
	 * and program above SIDs in below newly added SID registers in XUSB
	 * PADCTL MMIO space. These registers are TZ protected and so need to
	 * be done in ATF.
	 * a) #define XUSB_PADCTL_HOST_AXI_STREAMID_PF_0 (0x136cU)
	 * b) #define XUSB_PADCTL_DEV_AXI_STREAMID_PF_0  (0x139cU)
	 * c) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_0 (0x1370U)
	 * d) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_1 (0x1374U)
	 * e) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_2 (0x1378U)
	 * f) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_3 (0x137cU)
	 *
	 * This change disables SID override and programs XUSB SIDs in
	 * above registers to support both virtualization and non-virtualization
	 *
	 * Known Limitations:
	 * If xusb interface disables SMMU in XUSB DT in non-virtualization
	 * setup then there will be SMMU fault. We need to use WAR at
	 * https://git-master.nvidia.com/r/1529227/ to the issue.
	 *
	 * More details can be found in the bug 1971161
	 */
	mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
		XUSB_PADCTL_HOST_AXI_STREAMID_PF_0, TEGRA_SID_XUSB_HOST);
	mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
		XUSB_PADCTL_HOST_AXI_STREAMID_VF_0, TEGRA_SID_XUSB_VF0);
	mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
		XUSB_PADCTL_HOST_AXI_STREAMID_VF_1, TEGRA_SID_XUSB_VF1);
	mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
		XUSB_PADCTL_HOST_AXI_STREAMID_VF_2, TEGRA_SID_XUSB_VF2);
	mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
		XUSB_PADCTL_HOST_AXI_STREAMID_VF_3, TEGRA_SID_XUSB_VF3);
	mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
		XUSB_PADCTL_DEV_AXI_STREAMID_PF_0, TEGRA_SID_XUSB_DEV);
}

/* Secure IRQs for Tegra186 */
static const irq_sec_cfg_t tegra186_sec_irqs[] = {
	[0] = {
		TEGRA186_BPMP_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[1] = {
		TEGRA186_BPMP_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[2] = {
		TEGRA186_SPE_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[3] = {
		TEGRA186_SCE_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[4] = {
		TEGRA186_TOP_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[5] = {
		TEGRA186_AON_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
};

/*******************************************************************************
 * Initialize the GIC and SGIs
 ******************************************************************************/
void plat_gic_setup(void)
{
	tegra_gic_setup(tegra186_sec_irqs,
		sizeof(tegra186_sec_irqs) / sizeof(tegra186_sec_irqs[0]));

	/*
	 * Initialize the FIQ handler only if the platform supports any
	 * FIQ interrupt sources.
	 */
	if (sizeof(tegra186_sec_irqs) > 0)
		tegra_fiq_handler_setup();
}

/*******************************************************************************
 * Return pointer to the BL31 params from previous bootloader
 ******************************************************************************/
struct tegra_bl31_params *plat_get_bl31_params(void)
{
	uint32_t val;

	val = mmio_read_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV53_LO);

	return (struct tegra_bl31_params *)(uintptr_t)val;
}

/*******************************************************************************
 * Return pointer to the BL31 platform params from previous bootloader
 ******************************************************************************/
plat_params_from_bl2_t *plat_get_bl31_plat_params(void)
{
	uint32_t val;

	val = mmio_read_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV53_HI);

	return (plat_params_from_bl2_t *)(uintptr_t)val;
}
