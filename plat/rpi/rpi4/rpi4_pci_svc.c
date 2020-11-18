/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/pmf/pmf.h>
#include <lib/runtime_instr.h>
#include <services/sdei.h>
#include <services/std_svc.h>
#include <smccc_helpers.h>

#include <lib/mmio.h>

spinlock_t pci_lock;

#define PCIE_REG_BASE		 0xfd500000
#define PCIE_EXT_CFG_INDEX	 0x9000
/* A small window pointing at the ECAM of the device selected by CFG_INDEX */
#define PCIE_EXT_CFG_DATA	 0x8000
#define INVALID_PCI_ADDR	 0xFFFFFFFF


#define	 PCIE_EXT_BUS_SHIFT				20
#define	 PCIE_EXT_DEV_SHIFT				15
#define	 PCIE_EXT_FUN_SHIFT				12


static uint64_t pci_segment_lib_get_base (uint32_t address, uint32_t offset)
{
	uint64_t	  base;
	uint32_t	  bus, dev, fun;

	base = PCIE_REG_BASE;

	offset &= 0xFFF;	   /* Pick off the 4k register offset */

	/* The root port is at the base of the PCIe register space */
	if (address != 0U) {
		/* The current device is at CFG_DATA */
		base += PCIE_EXT_CFG_DATA;
		bus = PCI_ADDR_BUS (address);
		dev = PCI_ADDR_DEV (address);
		fun = PCI_ADDR_FUN (address);
		address = (bus << PCIE_EXT_BUS_SHIFT) | (dev << PCIE_EXT_DEV_SHIFT) | (fun << PCIE_EXT_FUN_SHIFT);
		/*
		 * Scan things out directly rather than translating the "bus" to a device, etc..
		 * only we need to limit each bus to a single device.
		 */
		if (dev < 1) {
			mmio_write_32 (PCIE_REG_BASE + PCIE_EXT_CFG_INDEX, address);
		} else {
			return INVALID_PCI_ADDR;
		}
	}
	return base + offset;
}

uint32_t pci_read_config(uint64_t addr, uint64_t off, uint64_t sz, uint32_t *val)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;
	uint64_t base;

	*val = INVALID_PCI_ADDR;
	spin_lock(&pci_lock);
	base = pci_segment_lib_get_base(addr, off);

	if (base == INVALID_PCI_ADDR) {
		*val = base;
	} else switch (sz) {
	case 1: //8 bits
		*val = mmio_read_8(base);
		break;
	case 2: //16 bits
		*val = mmio_read_16(base);
		break;
	case 4: //32 bits
		*val = mmio_read_32(base);
		break;
	default:
		ret = SMC_PCI_CALL_INVAL_PARAM;
	}
	spin_unlock(&pci_lock);
	return ret;
}

uint32_t pci_write_config(uint32_t addr, uint32_t off, uint32_t sz, uint32_t val)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;
	uint64_t base;

	spin_lock(&pci_lock);
	base = pci_segment_lib_get_base(addr, off);

	if (base == INVALID_PCI_ADDR) {
		ret = SMC_PCI_CALL_INVAL_PARAM;
	}
	else switch (sz) {
	case 1: //8 bits
		mmio_write_8(base, val);
		break;
	case 2: //16 bits
		mmio_write_16(base, val);
		break;
	case 4: //32 bits
		mmio_write_32(base, val);
		break;
	default:
		ret = SMC_PCI_CALL_INVAL_PARAM;
	}
	spin_unlock(&pci_lock);
	return ret;
}

uint32_t pci_get_bus_for_seg(uint32_t seg)
{
	return 0xFF00; //start bus 0, end bus 255
}
