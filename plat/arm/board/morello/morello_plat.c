/*
 * Copyright (c) 2020-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/arm/sbsa.h>
#include <plat/arm/common/plat_arm.h>
#include <smccc_helpers.h>

#include "morello_def.h"

/*
 * Table of regions to map using the MMU.
 * Replace or extend the below regions as required
 */
#if IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	MORELLO_MAP_DEVICE,
	MORELLO_MAP_NS_SRAM,
	ARM_MAP_DRAM1,
	ARM_MAP_DRAM2,
	{0}
};
#endif

#if IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	MORELLO_MAP_DEVICE,
	MORELLO_MAP_NS_SRAM,
	{0}
};
#endif

#if IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	MORELLO_MAP_DEVICE,
	MORELLO_MAP_NS_SRAM,
	ARM_MAP_DRAM1,
	ARM_MAP_DRAM2,
#if TRUSTED_BOARD_BOOT && !RESET_TO_BL2
	ARM_MAP_BL1_RW,
#endif
	{0}
};
#endif

#if TRUSTED_BOARD_BOOT
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	assert(heap_addr != NULL);
	assert(heap_size != NULL);

	return arm_get_mbedtls_heap(heap_addr, heap_size);
}
#endif

void plat_arm_secure_wdt_start(void)
{
	sbsa_wdog_start(SBSA_SECURE_WDOG_BASE, SBSA_SECURE_WDOG_TIMEOUT);
}

void plat_arm_secure_wdt_stop(void)
{
	sbsa_wdog_stop(SBSA_SECURE_WDOG_BASE);
}

/* SiP handler specific to each Arm platform. */
uintptr_t arm_plat_sip_handler(uint32_t smc_fid,
				u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *cookie,
				void *handle,
				u_register_t flags)
{
	WARN("Unimplemented ARM SiP Service Call: 0x%x \n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}
