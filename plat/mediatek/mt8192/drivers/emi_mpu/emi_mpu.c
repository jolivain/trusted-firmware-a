/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <emi_mpu.h>
#include <lib/mmio.h>

/*
 * emi_mpu_set_region_protection: protect a region.
 * @start: start address of the region
 * @end: end address of the region
 * @access_permission: EMI MPU access permission
 * Return 0 for success, otherwise negative status code.
 */
static int _emi_mpu_set_protection(
	unsigned long start, unsigned long end,
	unsigned int apc)
{
	unsigned int dgroup;
	unsigned int region;

	region = (start >> 24) & 0xFF;
	start &= 0x00FFFFFF;
	dgroup = (end >> 24) & 0xFF;
	end &= 0x00FFFFFF;

	if  ((region >= EMI_MPU_REGION_NUM) || (dgroup > EMI_MPU_DGROUP_NUM)) {
		WARN("Region:%u or dgroup:%u is wrong!\n", region, dgroup);
		return -1;
	}

	apc &= 0x80FFFFFF;

	if ((start >= DRAM_OFFSET) && (end >= start)) {
		start -= DRAM_OFFSET;
		end -= DRAM_OFFSET;
	} else {
		WARN("start:0x%lx or end:0x%lx address is wrong!\n",
		     start, end);
		return -2;
	}

	mmio_write_32(EMI_MPU_SA(region), start);
	mmio_write_32(EMI_MPU_EA(region), end);
	mmio_write_32(EMI_MPU_APC(region, dgroup), apc);

	return 0;
}

void dump_emi_mpu_regions(void)
{
	unsigned long apc[EMI_MPU_DGROUP_NUM], sa, ea;

	int region, i;

	/* Only dump 8 regions(max: EMI_MPU_REGION_NUM --> 32) */
	for (region = 0; region < 8; ++region) {
		for (i = 0; i < EMI_MPU_DGROUP_NUM; ++i)
			apc[i] = mmio_read_32(EMI_MPU_APC(region, i));
		sa = mmio_read_32(EMI_MPU_SA(region));
		ea = mmio_read_32(EMI_MPU_EA(region));

		WARN("region %d:\n", region);
		WARN("\tsa:0x%lx, ea:0x%lx, ",
		     sa, ea);
		WARN("apc0: 0x%lx apc1: 0x%lx", apc[0], apc[1]);
		WARN("\n");
	}
}

int emi_mpu_set_protection(struct emi_region_info_t *region_info)
{
	unsigned int start, end;
	int i;

	if (region_info->region >= EMI_MPU_REGION_NUM)
		return -1;

	start = (unsigned int)(region_info->start >> EMI_MPU_ALIGN_BITS) |
		(region_info->region << 24);

	for (i = EMI_MPU_DGROUP_NUM - 1; i >= 0; i--) {
		end = (unsigned int)(region_info->end >> EMI_MPU_ALIGN_BITS) |
			(i << 24);
		_emi_mpu_set_protection(start, end, region_info->apc[i]);
	}

	return 0;
}

void emi_mpu_init(void)
{
	/* Set permission */
	struct emi_region_info_t region_info;
#if 0	/* legacy usage */
	emi_mpu_set_region_protection(0x40000000UL, 0x4FFFFFFFUL, 0,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	emi_mpu_set_region_protection(0x50000000UL, 0x528FFFFFUL, 1,
				(FORBIDDEN << 6));
	emi_mpu_set_region_protection(0x52900000UL, 0x5FFFFFFFUL, 2,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	emi_mpu_set_region_protection(0x60000000UL, 0xFFFFFFFFUL, 3,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	emi_mpu_set_region_protection(0x100000000UL, 0x23FFFFFFFUL, 4,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
#endif

	region_info.start = 0x80000000;
	region_info.end = 0x80400000;
	region_info.region = 1;
	SET_ACCESS_PERMISSION(region_info.apc, 1,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      NO_PROTECTION /*FORBIDDEN*/);
	emi_mpu_set_protection(&region_info);

	dump_emi_mpu_regions();
}

