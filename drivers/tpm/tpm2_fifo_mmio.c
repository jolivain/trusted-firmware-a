/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/tpm/tpm2.h>
#include <platform_def.h>
#include <drivers/tpm/tpm2_chip.h>
#include <lib/mmio.h>

static uint32_t tpm2_read(uintptr_t addr, uint8_t len)
{
	if (len == BYTE)
		return *(volatile unsigned char *)(addr);
	else if (len == WORD)
		return *(volatile uint32_t *)addr;
	return -1;
}

static uint32_t tpm2_write(uintptr_t addr, uint8_t len, uint32_t value)
{
	if (len == BYTE)
		*(volatile unsigned char *)(addr) = value;
	else if (len == WORD)
		*(volatile uint32_t *)addr = value;
	return 0;
}

uint32_t tpm2_fifo_io(uintptr_t addr, uint8_t rw, uint8_t len, uint8_t val)
{
	uint8_t result;
	int ret = -1;

	switch (rw) {
	case TPM_READ:
		ret = tpm2_read(addr, len);
	break;
	case TPM_WRITE:
		ret = tpm2_write(addr, len, val);
	break;
	}
	return ret;
}

uint8_t tpm2_get_reg_data(uintptr_t addr, uint8_t *status)
{
	*status = *(volatile unsigned char *)(addr);
	return 0;
}
