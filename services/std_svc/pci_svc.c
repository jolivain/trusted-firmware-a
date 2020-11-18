/*
 * Copyright (c) 2014-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <services/pci_svc.h>
#include <services/sdei.h>
#include <services/std_svc.h>
#include <smccc_helpers.h>

static uint64_t validate_rw_addr_sz(uint64_t addr, uint64_t off, uint64_t sz)
{
	uint32_t nseg;
	uint32_t ret;
	uint32_t start_end_bus;

	ret = pci_get_bus_for_seg(PCI_ADDR_SEG(addr), &start_end_bus, &nseg);

	if (ret != SMC_PCI_CALL_SUCCESS) {
		return SMC_PCI_CALL_INVAL_PARAM;
	}
	switch (sz) {
	case SMC_PCI_SZ_8BIT:
	case SMC_PCI_SZ_16BIT:
	case SMC_PCI_SZ_32BIT:
		break;
	default:
		return SMC_PCI_CALL_INVAL_PARAM;
	}
	if ((off + sz) > PCI_OFFSET_MASK) {
		return SMC_PCI_CALL_INVAL_PARAM;
	}
	return SMC_PCI_CALL_SUCCESS;
}

uint64_t pci_smc_handler(uint32_t smc_fid,
			     u_register_t x1,
			     u_register_t x2,
			     u_register_t x3,
			     u_register_t x4,
			     void *cookie,
			     void *handle,
			     u_register_t flags)
{
	switch (smc_fid) {
	case SMC_PCI_VERSION:	{
		pcie_version ver;

		ver.Major = 1U;
		ver.Minor = 0U;
		SMC_RET1(handle, ver.val);
		break;
	}
	case SMC_PCI_FEATURES:
		switch (x1) {
		case SMC_PCI_VERSION:
		case SMC_PCI_FEATURES:
		case SMC_PCI_READ:
		case SMC_PCI_WRITE:
		case SMC_PCI_SEG_INFO:
			SMC_RET1(handle, 0);
			break;
		default:
			SMC_RET1(handle, SMC_PCI_CALL_NOT_SUPPORTED);
		}
		break;
	case SMC_PCI_READ: {
		uint32_t ret;

		if (validate_rw_addr_sz(x1, x2, x3) != SMC_PCI_CALL_SUCCESS) {
			SMC_RET2(handle, SMC_PCI_CALL_INVAL_PARAM, 0);
		}
		if (x4 != 0U) {
			SMC_RET2(handle, SMC_PCI_CALL_INVAL_PARAM, 0);
		}
		if (pci_read_config(x1, x2, x3, &ret) != 0U) {
			SMC_RET2(handle, SMC_PCI_CALL_INVAL_PARAM, 0);
		} else {
			SMC_RET2(handle, 0,  ret);
		}
		break;
	}
	case SMC_PCI_WRITE: {
		uint32_t ret;

		if (validate_rw_addr_sz(x1, x2, x3) != SMC_PCI_CALL_SUCCESS) {
			SMC_RET1(handle, SMC_PCI_CALL_INVAL_PARAM);
		}
		ret = pci_write_config(x1, x2, x3, x4);
		SMC_RET1(handle, ret);
		break;
	}
	case SMC_PCI_SEG_INFO: {
		uint32_t nseg;
		uint32_t ret;
		uint32_t start_end_bus;

		if ((x2 != 0U) || (x3 != 0U) || (x4 != 0U)) {
			SMC_RET1(handle, SMC_PCI_CALL_INVAL_PARAM);
		}
		ret = pci_get_bus_for_seg(x1, &start_end_bus, &nseg);
		SMC_RET3(handle, ret, start_end_bus, nseg);
		break;
	}
	default:
		/* should be unreachable */
		WARN("Unimplemented PCI Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_PCI_CALL_NOT_SUPPORTED);
	}
}
