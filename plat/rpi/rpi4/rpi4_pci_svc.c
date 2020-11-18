/*
 * Copyright (c) 2014-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/pmf/pmf.h>
#include <lib/runtime_instr.h>
#include <services/pci_svc.h>
#include <services/sdei.h>
#include <services/std_svc.h>
#include <smccc_helpers.h>

#include <lib/mmio.h>

static spinlock_t pci_lock;

#define PCIE_REG_BASE		0xfd500000
#define PCIE_EXT_CFG_INDEX	0x9000
/* A small window pointing at the ECAM of the device selected by CFG_INDEX */
#define PCIE_EXT_CFG_DATA	0x8000
#define INVALID_PCI_ADDR	0xFFFFFFFF


#define	 PCIE_EXT_BUS_SHIFT	20
#define	 PCIE_EXT_DEV_SHIFT	15
#define	 PCIE_EXT_FUN_SHIFT	12


static uint64_t pci_segment_lib_get_base(uint32_t address, uint32_t offset)
{
	uint64_t	  base;
	uint32_t	  bus, dev, fun;

	base = PCIE_REG_BASE;

	offset &= PCI_OFFSET_MASK;  /* Pick off the 4k register offset */

	/* The root port is at the base of the PCIe register space */
	if (address != 0U) {
		/* The current device is at CFG_DATA */
		base += PCIE_EXT_CFG_DATA;
		bus = PCI_ADDR_BUS(address);
		dev = PCI_ADDR_DEV(address);
		fun = PCI_ADDR_FUN(address);
		address = (bus << PCIE_EXT_BUS_SHIFT) |
			  (dev << PCIE_EXT_DEV_SHIFT) |
			  (fun << PCIE_EXT_FUN_SHIFT);
		/* RPi weirdness, only handle a single dev on root bus */
		if ((bus == 0U) && (dev > 0U)) {
			return INVALID_PCI_ADDR;
		}
		mmio_write_32(PCIE_REG_BASE + PCIE_EXT_CFG_INDEX, address);
	}
	return base + offset;
}

/**
 * pci_read_config() - Performs a config space read at addr
 * @addr: 32-bit, segment, BDF of requested function.
 * @off:  register offset of function described by @addr to read
 * @sz:	  size of read (8,16,32) bits.
 * @val:  returned zero extended value read from config space
 *
 * sz bits of PCI config space is read at addr:offset, and the value
 * is returned in val. Invalid segment/offset values return failure.
 * Reads to valid functions that don't exist return INVALID_PCI_ADDR
 * as is specified by PCI for requests that aren't completed by EPs.
 * The boilerplate in pci_svc.c tends to do basic segment, off
 * and sz validation. This routine should avoid duplicating those
 * checks.
 *
 * This function maps directly to the PCI_READ function in DEN0115
 * where detailed requirements may be found.
 *
 * Return: SMC_PCI_CALL_SUCCESS with val set
 *	   SMC_PCI_CALL_INVAL_PARAM, on parameter error
 */
uint32_t pci_read_config(uint64_t addr, uint64_t off, uint64_t sz, uint32_t *val)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;
	uint64_t base;

	*val = 0;

	spin_lock(&pci_lock);
	base = pci_segment_lib_get_base(addr, off);

	if (base == INVALID_PCI_ADDR) {
		*val = base;
	} else {
		switch (sz) {
		case SMC_PCI_SZ_8BIT:
			*val = mmio_read_8(base);
			break;
		case SMC_PCI_SZ_16BIT:
			*val = mmio_read_16(base);
			break;
		case SMC_PCI_SZ_32BIT:
			*val = mmio_read_32(base);
			break;
		default: /* should be unreachable */
			ret = SMC_PCI_CALL_INVAL_PARAM;
		}
	}
	spin_unlock(&pci_lock);
	return ret;
}

/**
 * pci_write_config() - Performs a config space write at addr
 * @addr: 32-bit, segment, BDF of requested function.
 * @off:  register offset of function described by @addr to write
 * @sz:	  size of write (8,16,32) bits.
 * @val:  value to be written
 *
 * sz bits of PCI config space is written at addr:offset. Invalid
 * segment/BDF values return failure. Writes to valid functions
 * without valid EPs are ignored, as is specified by PCI.
 * The boilerplate in pci_svc.c tends to do basic segment, off
 * and sz validation, so it shouldn't need to be repeated here.
 *
 * This function maps directly to the PCI_WRITE function in DEN0115
 * where detailed requirements may be found.
 *
 * Return: SMC_PCI_CALL_SUCCESS
 *	   SMC_PCI_CALL_INVAL_PARAM, on parameter error
 */
uint32_t pci_write_config(uint32_t addr, uint32_t off, uint32_t sz, uint32_t val)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;
	uint64_t base;

	spin_lock(&pci_lock);
	base = pci_segment_lib_get_base(addr, off);

	if (base != INVALID_PCI_ADDR) {
		switch (sz) {
		case SMC_PCI_SZ_8BIT:
			mmio_write_8(base, val);
			break;
		case SMC_PCI_SZ_16BIT:
			mmio_write_16(base, val);
			break;
		case SMC_PCI_SZ_32BIT:
			mmio_write_32(base, val);
			break;
		default: /* should be unreachable */
			ret = SMC_PCI_CALL_INVAL_PARAM;
		}
	}
	spin_unlock(&pci_lock);
	return ret;
}

/**
 * pci_get_bus_for_seg() - returns the start->end bus range for a segment
 * @seg:  segment being queried
 * @bus_range:	returned bus begin + (end << 8)
 * @nseg: returns next segment in this machine or 0 for end
 *
 * pci_get_bus_for_seg is called to check if a given segment is
 * valid on this machine. If it is valid, then its bus ranges are
 * returned along with the next valid segment on the machine. If
 * this is the last segment, then nseg must be 0.
 *
 * This function maps directly to the PCI_GET_SEG_INFO function
 * in DEN0115 where detailed requirements may be found.
 *
 * Return: SMC_PCI_CALL_SUCCESS, and appropriate bus_range and nseg
 *	   SMC_PCI_CALL_NOT_IMPL, if the segment is invalid
 */
uint32_t pci_get_bus_for_seg(uint32_t seg, uint32_t *bus_range, uint32_t *nseg)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;
	*nseg = 0U; /* only a single segment */
	if (seg == 0U) {
		*bus_range = 0xFF00; /* start 0, end 255 */
	} else {
		*bus_range = 0U;
		ret = SMC_PCI_CALL_NOT_IMPL;
	}
	return ret;
}
