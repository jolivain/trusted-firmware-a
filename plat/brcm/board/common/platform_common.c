/*
 * Copyright (c) 2015-2019, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/brcm/sotp.h>

#include <cmn_plat_util.h>
#include <platform_def.h>

#define BOOT_SOURCE_SECONDARY_ENABLE_OFFSET 15
#define BOOT_SOURCE_SECONDARY_ENABLE_MASK \
	(1 << BOOT_SOURCE_SECONDARY_ENABLE_OFFSET)

#ifndef PLAT_SECONDARY_BOOT_SOURCE
#define PLAT_SECONDARY_BOOT_SOURCE BOOT_SOURCE_UNKNOWN
#endif

#pragma weak bcm_reserve_mem

void bcm_reserve_mem(uintptr_t *free_base, size_t *free_size,
		     uintptr_t addr, size_t size)
{
}

uint32_t boot_source_get(void)
{
	uint32_t data;

#ifdef FORCE_BOOTSOURCE
	data = FORCE_BOOTSOURCE;
#else
	/* Read primary boot strap from CRMU persistent registers */
	data = mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG1);
	if (data & BOOT_SOURCE_SOFT_ENABLE_MASK) {
		data >>= BOOT_SOURCE_SOFT_DATA_OFFSET;
	} else {
		uint64_t sotp_atf_row;

		sotp_atf_row =
			sotp_mem_read(SOTP_ATF_CFG_ROW_ID, SOTP_ROW_NO_ECC);

		if (sotp_atf_row & SOTP_BOOT_SOURCE_ENABLE_MASK) {
			/* Construct the boot source based on SOTP bits */
			data = 0;
			if (sotp_atf_row & SOTP_BOOT_SOURCE_BITS0)
				data |= 0x1;
			if (sotp_atf_row & SOTP_BOOT_SOURCE_BITS1)
				data |= 0x2;
			if (sotp_atf_row & SOTP_BOOT_SOURCE_BITS2)
				data |= 0x4;
		} else {

			/*
			 * This path is for L0 reset with
			 * Primary Boot source disabled in SOTP.
			 * BOOT_SOURCE_FROM_PR_ON_L1 compile flag will allow
			 * to never come back here so that the
			 * external straps will not be read on L1 reset.
			 */

			/* Use the external straps */
			data = mmio_read_32(ROM_S0_IDM_IO_STATUS);

#ifdef BOOT_SOURCE_FROM_PR_ON_L1
			/* Enable boot source read from PR#1 */
			mmio_setbits_32(CRMU_IHOST_SW_PERSISTENT_REG1,
				BOOT_SOURCE_SOFT_ENABLE_MASK);

			/* set boot source */
			data &= BOOT_SOURCE_MASK;
			mmio_clrsetbits_32(CRMU_IHOST_SW_PERSISTENT_REG1,
			BOOT_SOURCE_MASK << BOOT_SOURCE_SOFT_DATA_OFFSET,
			data << BOOT_SOURCE_SOFT_DATA_OFFSET);
#endif
		}
	}
#endif
	return (data & BOOT_SOURCE_MASK);
}

/* Platform can override the bypassed reserve_mem as needed. */
void reserve_mem(uintptr_t *free_base, size_t *free_size,
		 uintptr_t addr, size_t size)
{
	bcm_reserve_mem(free_base, free_size, addr, size);
}

void __dead2 plat_soft_reset(uint32_t reset)
{
	if (reset == SOFT_RESET_L3) {
		mmio_setbits_32(CRMU_IHOST_SW_PERSISTENT_REG1, reset);
		mmio_write_32(CRMU_MAIL_BOX0, 0x0);
		mmio_write_32(CRMU_MAIL_BOX1, 0xFFFFFFFF);
	}

	if (reset != SOFT_SYS_RESET_L1)
		reset = SOFT_PWR_UP_RESET_L0;

	if (reset == SOFT_PWR_UP_RESET_L0)
		INFO("L0 RESET...\n");

	if (reset == SOFT_SYS_RESET_L1)
		INFO("L1 RESET...\n");

	console_flush();

	mmio_clrbits_32(CRMU_SOFT_RESET_CTRL, 1 << reset);

	while (1)
		;
}

static uint32_t secondary_boot_get(void)
{
	uint64_t sotp_row;
	uint32_t secondary_boot;
	uint32_t boot_source = PLAT_SECONDARY_BOOT_SOURCE;

	sotp_row = sotp_mem_read(SOTP_DEVICE_SECURE_CFG2_ROW, 0);

	secondary_boot =
		sotp_redundancy_reduction((uint32_t)((sotp_row >> 6) & 0x3));

	if (secondary_boot) {

		secondary_boot =
		sotp_redundancy_reduction((uint32_t)((sotp_row >> 0) & 0x3f));

		INFO("Secondary boot source: %d\n", secondary_boot);
		boot_source = secondary_boot & BOOT_SOURCE_MASK;
	}

	return boot_source;
}

void halt_boot_check(void)
{
	uint64_t sotp_row;

	/* Check SOTP to know if we need to not halt */
	sotp_row = sotp_mem_read(SOTP_ATF_CFG_ROW_ID, 0);

	if ((sotp_row & SOTP_ATF_RESET_RETRY) == 0) {
		dsb();
		/* Loop until watchdog (if enabled) resets the system */
		for (;;)
			wfi();
	}
}

void plat_secondary_boot_source(void)
{
	uint32_t secondary_boot_source;

	/*
	 * Secondary boot use case: primary boot source in SOTP
	 * is used instead of boot straps (external to the chip).
	 * If secondary boot source is enabled in SOTP,
	 * set soft straps override to SOTP secondary boot source and reset.
	 */

	/* Read secondary boot source */
	secondary_boot_source = secondary_boot_get();

	if (secondary_boot_source != BOOT_SOURCE_UNKNOWN) {

		/*
		 * If this boot was not from secondary boot source,
		 * load from secondary boot source.
		 */
		if ((mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG1) &
				BOOT_SOURCE_SECONDARY_ENABLE_MASK) == 0) {

			INFO("Restart from boot source: %d\n",
				secondary_boot_source);

			/*
			 * Enable Soft boot source and advertise we will be
			 * running from secondary path.
			 */
			mmio_setbits_32(CRMU_IHOST_SW_PERSISTENT_REG1,
			BOOT_SOURCE_SOFT_ENABLE_MASK |
			BOOT_SOURCE_SECONDARY_ENABLE_MASK);

			/* set next boot source */
			mmio_clrsetbits_32(CRMU_IHOST_SW_PERSISTENT_REG1,
			BOOT_SOURCE_MASK << BOOT_SOURCE_SOFT_DATA_OFFSET,
			secondary_boot_source << BOOT_SOURCE_SOFT_DATA_OFFSET);

			/* Restart */
			console_flush();

			/* House cleaning for BL1 restart */
			disable_mmu_el3();
			flush_dcache_range(BL1_RW_BASE,
					BL1_RW_LIMIT - BL1_RW_BASE);

			((void (*)(void))PLAT_BRCM_TRUSTED_ROM_BASE)();
		}
	}

	halt_boot_check();
}
