/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <drivers/arm/sbsa.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

void sbsa_watchdog_offset_reg_write(uintptr_t base, uint64_t value)
{
	assert((value >> SBSA_WDOG_WOR_WIDTH) == 0);
	mmio_write_32(base + SBSA_WDOG_WOR_LOW_OFFSET,
		 ((uint32_t)value & UINT32_MAX));
	mmio_write_32(base + SBSA_WDOG_WOR_HIGH_OFFSET, (uint32_t)(value >> 32));
}

/*
 * Start the watchdog timer at base address "base" for a
 * period of "ms" milliseconds.The watchdog has to be
 * refreshed within this time period.
 */
void sbsa_wdog_start(uintptr_t base, uint64_t ms)
{
	uint64_t counter_freq;
	uint64_t offset_reg_value;

	counter_freq = (uint64_t)plat_get_syscnt_freq2();
	offset_reg_value = ms * counter_freq / 1000;

	sbsa_watchdog_offset_reg_write(base, offset_reg_value);
	mmio_write_32(base + SBSA_WDOG_WCS_OFFSET, SBSA_WDOG_WCS_EN);
}

/* Stop the watchdog */
void sbsa_wdog_stop(uintptr_t base)
{
	mmio_write_32(base + SBSA_WDOG_WCS_OFFSET, (0x0));
}

#if SBSA_WDOG_HANDLING

#if defined(PLAT_SBSA_TWDG_PRI)
#define weak plat_sbsa_twdg_ws0_handler
static int plat_sbsa_twdg_ws0_handler(unsigned int id, unsigned int flags,
				      void *handle, void *cookie)
{
	ERROR("sbsa secure watchdog WS0");
	panic();
	return 0;
}
#endif

#define weak plat_sbsa_wdog_ws1_handler

unsigned int plat_sbsa_wdog_ws1_handler(void *cookie)
{
	return PSCI_RESET2_SYSTEM_WARM_RESET;
}

static int sbsa_ntwdg_ws1_handler(unsigned int id, unsigned int flags,
				  void *handle, void *cookie)
{
	unsigned int reset_type = PSCI_RESET2_SYSTEM_WARM_RESET;

	reset_type = plat_sbsa_ntwdg_ws1_handler(cookie);
	psci_system_reset2(reset_type, cookie);

	return 0;
}

void sbsa_wdog_handler_init(void)
{
#if EL3_EXCEPTION_HANDLING
	ehf_register_priority_handler(PLAT_SBSA_NTWDG_PRI, sbsa_ntwdg_ws1_handler);
#if defined(PLAT_SBSA_TWDG_PRI)
	ehf_register_priority_handler(PLAT_SBSA_TWDG_PRI, plat_sbsa_twdg_ws0_handler);
#endif
#endif
}
#endif
