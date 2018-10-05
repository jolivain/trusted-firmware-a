/*
 * Copyright (c) 2021, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <libfdt.h>

#include <common/debug.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <common/tf_crc32.h>

#include <lib/mmio.h>

#include <sunxi_mmap.h>
#include <sunxi_private.h>

#ifdef SUNXI_GENERATE_MAC_ADDRESSES
/*
 * Generate a MAC address based on the SID ID numbers.
 * The SID is a secure device holding, among other information, a few registers
 * with apparently unique serial numbers.
 * U-Boot has for long years used this to generate a MAC address.
 * If secure boot is enabled, U-Boot (running in EL2) has no longer access
 * to the secure SID device, so it fails to generate the MAC address.
 * Let us do this in EL3 instead, and write the generated number into the
 * DT below.
 * The algorithm here is identical to U-Boot's version, so is generating the
 * very same MAC address.
 */
static void sunxi_generate_mac_address(unsigned int index, uint8_t *mac_addr)
{
	uint32_t sid[4], crc;
	int i;

	for (i = 0; i < 4; i++) {
		sid[i] = mmio_read_32(SUNXI_SID_BASE + 0x200 + i * 4);
	}

	crc = tf_crc32(0, (uint8_t *)&sid[1], 12);

	mac_addr[0] = (index << 4) | BIT_32(1);
	mac_addr[1] = sid[0] & 0xff;
	mac_addr[2] = crc >> 24;
	mac_addr[3] = (crc >> 16) & 0xff;
	mac_addr[4] = (crc >> 8) & 0xff;
	mac_addr[5] = crc & 0xff;
}

static void fdt_add_mac_addresses(void *fdt)
{
	uint8_t mac_addr[6];
	int ret, index;

	/* Up to two MAC addresses, for Ethernet and WiFi. */
	for (index = 0; index < 2; index++) {
		sunxi_generate_mac_address(index, mac_addr);
		ret = fdt_set_mac_address(fdt, index, mac_addr);
		if (ret && ret != -FDT_ERR_NOTFOUND) {
			WARN("BL31: cannot write MAC address into device tree: %d\n",
			     ret);
		}
		if (ret) {
			return;
		}

		INFO("BL31: MAC address #%d: %02x:%02x:%02x:%02x:%02x:%02x\n",
		     index, mac_addr[0], mac_addr[1], mac_addr[2],
		     mac_addr[3], mac_addr[4], mac_addr[5]);
	}
}

#else	/* SUNXI_GENERATE_MAC_ADDRESSES */
static void fdt_add_mac_addresses(void *fdt)
{
}
#endif

void sunxi_prepare_dtb(void *fdt)
{
	int ret;

	if (fdt == NULL || fdt_check_header(fdt) != 0) {
		return;
	}

	ret = fdt_open_into(fdt, fdt, 0x10000);
	if (ret < 0) {
		ERROR("Preparing devicetree at %p: error %d\n", fdt, ret);
		return;
	}

#ifdef SUNXI_BL31_IN_DRAM
	/* Reserve memory used by Trusted Firmware. */
	if (fdt_add_reserved_memory(fdt, "tf-a@40000000", BL31_BASE,
				    BL31_LIMIT - BL31_BASE)) {
		WARN("Failed to add reserved memory nodes to DT.\n");
		return;
	}
#endif

	fdt_add_mac_addresses(fdt);

	ret = fdt_pack(fdt);
	if (ret < 0) {
		ERROR("Failed to pack devicetree at %p: error %d\n",
		      fdt, ret);
	} else {
		clean_dcache_range((uintptr_t)fdt, fdt_blob_size(fdt));
		INFO("Changed devicetree.\n");
	}
}
