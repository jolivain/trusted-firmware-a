/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STD_SVC_H
#define STD_SVC_H

/* SMC function IDs for Standard Service queries */

#define ARM_STD_SVC_CALL_COUNT		0x8400ff00
#define ARM_STD_SVC_UID			0x8400ff01
/*					0x8400ff02 is reserved */
#define ARM_STD_SVC_VERSION		0x8400ff03

/* SPCI platform functions */
#define SMC_PCI_VERSION 		U(0x84000130)
#define SMC_PCI_FEATURES		U(0x84000131)
#define SMC_PCI_READ			U(0x84000132)
#define SMC_PCI_WRITE			U(0x84000133)
#define SMC_PCI_SEG_INFO		U(0x84000134)

#define PCI_ADDR_FUN(dev) (dev & 0x7)
#define PCI_ADDR_DEV(dev) ((dev >> 3) & 0x1F)
#define PCI_ADDR_BUS(dev) ((dev >> 8) & 0xFF)
typedef union
{
	struct {
		uint16_t Minor;
		uint16_t Major;
	} __packed;
	uint32_t val;
} pcie_version;

uint32_t pci_read_config(uint64_t addr, uint64_t off, uint64_t sz, uint32_t *val);
uint32_t pci_write_config(uint32_t addr, uint32_t off, uint32_t sz, uint32_t val);
uint32_t pci_get_bus_for_seg(uint32_t seg);

/* Return codes for Arm PCI Config Space Access Firmware SMC calls */
#define SMC_PCI_CALL_SUCCESS		0
#define SMC_PCI_CALL_NOT_SUPPORTED	-1
#define SMC_PCI_CALL_INVAL_PARAM	-2
#define SMC_PCI_CALL_NOT_IMPL		-3

/* ARM Standard Service Calls version numbers */
#define STD_SVC_VERSION_MAJOR		0x0
#define STD_SVC_VERSION_MINOR		0x1

/*
 * Get the ARM Standard Service argument from EL3 Runtime.
 * This function must be implemented by EL3 Runtime and the
 * `svc_mask` identifies the service. `svc_mask` is a bit
 * mask identifying the range of SMC function IDs available
 * to the service.
 */
uintptr_t get_arm_std_svc_args(unsigned int svc_mask);

#endif /* STD_SVC_H */
