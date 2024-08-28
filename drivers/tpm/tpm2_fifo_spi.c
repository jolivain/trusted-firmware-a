/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/tpm/tpm2.h>
#include <drivers/tpm/tpm2_chip.h>
#include <lib/mmio.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

extern struct spi_plat *spidev;

int tpm2_spi_transfer(const uint8_t *data_out, uint8_t *data_in, uint16_t len)
{
	return spidev->ops->xfer(len, data_out, data_in);
}

int tpm2_spi_start_transaction(uint32_t addr, int access, uint16_t len)
{
	int rc;
	uint8_t header[4];
	uint8_t header_response[4];
	uint8_t zero = 0, byte;
	int retries;

	/* check to make sure len does not exceed the encoding limit */
	if(len > 128) {
		return -1;
	}

	header[0] = ((access == TPM_READ) ? 0x80 : 0x00) | (len - 1);

	header[1] = 0xD4;
	header[2] = addr >> 8 ;
	header[3] = addr;

	rc = tpm2_spi_transfer(header, header_response, 4);
	if(rc != 0)
		return -1;

	byte = header_response[3];
	if (byte & 1)
		return 0;

	for (retries = 256; retries > 0; retries--) {
		rc = tpm2_spi_transfer(&zero, &byte, 1);
		if (rc != 0){
			return -1;
		}
		if (byte & 1)
			return 0;
	}

	if(retries == 0) {
		ERROR("TPM : Timeout\n");
		return -1;
	}

	return 0;
}

void tpm2_spi_end_transaction()
{
	spidev->ops->stop();
}

void tpm2_spi_init()
{
	spidev->ops->get_access();
	spidev->ops->start();
}

int tpm2_spi_read(uint32_t addr, uint8_t *in, uint16_t len)
{
	int rc;

	tpm2_spi_init();
	rc = tpm2_spi_start_transaction(addr, TPM_READ, len);
	if (rc != 0) {
		tpm2_spi_end_transaction();
		return -1;
	}

	rc = tpm2_spi_transfer(NULL, in, len);
	if (rc != 0) {
		tpm2_spi_end_transaction();
		return -1;
	}

	tpm2_spi_end_transaction();

	return 0;
}

int tpm2_spi_write(uint32_t addr, const uint8_t *out, uint16_t len)
{
	int rc;

	tpm2_spi_init();

	rc = tpm2_spi_start_transaction(addr, TPM_WRITE, len);
	if (rc != 0) {
		tpm2_spi_end_transaction();
		return -1;
	}

	rc = tpm2_spi_transfer(out, NULL, len);
	if (rc != 0) {
		tpm2_spi_end_transaction();
		return -1;
	}

	tpm2_spi_end_transaction();

	return 0;
}

uint32_t tpm2_fifo_io(uintptr_t addr, uint8_t rw, uint8_t len, uint8_t val)
{
	uint8_t result;
	int ret = -1;

	if(rw == TPM_READ) {
		ret = tpm2_spi_read((uint32_t)addr, &result, len);
		if(ret != 0)
			return ret;
		return result;
	} else if(rw == TPM_WRITE) {
		ret = tpm2_spi_write((uint32_t)addr, &val , len);
	}

	return ret;
}

uint8_t tpm2_get_reg_data(uintptr_t addr, uint8_t *status)
{
	return tpm2_spi_read((uint32_t)addr, status, BYTE);
}
