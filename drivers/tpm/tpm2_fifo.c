/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/tpm/tpm2.h>
#include <drivers/tpm/tpm2_chip.h>
#include <drivers/delay_timer.h>
#include <drivers/tpm/tpm2_interface.h>

#define TPM_TIMEOUT_MS 5

#define LOCALITY_START_ADDRESS(x, y) \
	((tpm_registers *)(x->address + (0x1000 * y)))

int tpm2_get_info(chip_data_t *chip_data, uint8_t locality)
{
	tpm_registers *tpm_regs = LOCALITY_START_ADDRESS(chip_data, locality);

	uint32_t vid_did = tpm2_fifo_io((uintptr_t)&tpm_regs->vendid,
				TPM_READ, WORD, 0);
	uint8_t revision = tpm2_fifo_io((uintptr_t)&tpm_regs->revid,
				TPM_READ, WORD, 0);

	NOTICE("TPM Chip: vendor-id 0x%x, device-id 0x%x, revision-id: 0x%x\n",
		0xFFFF & vid_did, vid_did >> 16, revision);

	return 0;
}

static int tpm2_wait_reg_bits(uintptr_t reg, uint8_t set, unsigned long timeout, uint8_t *status)
{
	int ret;
	uint64_t timeout_delay = plat_timeout_init_us(timeout * 1000);
	do {
		plat_mdelay(TPM_TIMEOUT_MS);
		ret = tpm2_get_reg_data(reg, status);
		if (ret != 0) {
			return ret;
		}
		if ((*status & set) == set)	{
			return 0;
		}
	} while (!plat_timeout_elapsed(timeout_delay));

	return -1;
}

int tpm2_fifo_request_access(chip_data_t *chip_data, uint8_t locality)
{
	tpm_registers *tpm_regs = LOCALITY_START_ADDRESS(chip_data, locality);
	uint8_t status;


	tpm2_fifo_io((uintptr_t)&tpm_regs->access,
			TPM_WRITE, BYTE, TPM_ACCESS_REQUEST_USE);
	int ret = tpm2_wait_reg_bits((uintptr_t)&tpm_regs->access,
					TPM_ACCESS_ACTIVE_LOCALITY,
					chip_data->timeout_msec_a, &status);
	if (ret == 0) {
		chip_data->locality = locality;
		return 0;
	}

	return -1;
}

void tpm2_fifo_release_locality(chip_data_t *chip_data, uint8_t locality)
{
	tpm_registers *tpm_regs = LOCALITY_START_ADDRESS(chip_data, locality);
	uint8_t buf;
	int ret = 0;

	buf = tpm2_fifo_io((uintptr_t)&tpm_regs->access, TPM_READ, BYTE, 0);
	if (buf < 0) {
		return;
	}

	if (buf & (TPM_ACCESS_REQUEST_PENDING | TPM_ACCESS_VALID)) {
		ret = tpm2_fifo_io((uintptr_t)&tpm_regs->access,
				TPM_WRITE, BYTE, TPM_ACCESS_RELINQUISH_LOCALITY);
	}
	if(ret < 0) {
		ERROR("TPM: Unable to release locality\n");
	}
}

static int tpm2_fifo_prepare(chip_data_t *chip_data)
{
	tpm_registers *tpm_regs =
			LOCALITY_START_ADDRESS(chip_data, chip_data->locality);
	uint8_t status;

	tpm2_fifo_io((uintptr_t)&tpm_regs->status,
			TPM_WRITE, BYTE, TPM_STAT_COMMAND_READY);

	int ret = tpm2_wait_reg_bits((uintptr_t)&tpm_regs->status,
				TPM_STAT_COMMAND_READY,
				chip_data->timeout_msec_b, &status);
	if (ret < 0) {
		ERROR("TPM : Status Busy\n");
		return -1;
	}

	return 0;
}

uint16_t tpm2_fifo_read_burstcount(chip_data_t *chip_data)
{
	uint16_t burstcount;
	tpm_registers *tpm_regs =
			LOCALITY_START_ADDRESS(chip_data, chip_data->locality);
	uint64_t timeout_delay = plat_timeout_init_us(chip_data->timeout_msec_a * 1000);

	do {
		uint8_t byte0, byte1;
		plat_mdelay(TPM_TIMEOUT_MS);

		byte0 = tpm2_fifo_io((uintptr_t)&tpm_regs->burst_count,
				TPM_READ, BYTE, 0);
		byte1 = tpm2_fifo_io((uintptr_t)&tpm_regs->burst_count + 1,
				TPM_READ, BYTE, 0);

		burstcount = (uint16_t)((byte1 << 8) + byte0);
		if (burstcount)
			return burstcount;
	} while (!plat_timeout_elapsed(timeout_delay));

	return -1;
}

int tpm2_fifo_send(chip_data_t *chip_data, const uint8_t *buf)
{
	uint8_t status;
	uint16_t burstcnt;
	uint32_t ret, len, index = 0;
	const uint16_t *length_offset = (uint16_t *)(buf+2);
	tpm_registers *tpm_regs =
			LOCALITY_START_ADDRESS(chip_data, chip_data->locality);

	tpm2_get_reg_data((uintptr_t)&tpm_regs->status, &status);


	if (!(status & TPM_STAT_COMMAND_READY)) {
		ret = tpm2_fifo_prepare(chip_data);
		if (ret < 0)
			return -1;
	}

	len =  bswap32(*length_offset | *(length_offset + 1) << 16);

	while (index < len) {
		burstcnt = tpm2_fifo_read_burstcount(chip_data);
		if (burstcnt < 0)
			return -1;

		for (; burstcnt > 0 && index < len; burstcnt--) {
			ret = tpm2_fifo_io((uintptr_t)&tpm_regs->data_fifo,
					TPM_WRITE, BYTE, *(buf + index));
			if (ret < 0) {
				return -1;
			}
			index++;
		}
	}

	ret = tpm2_wait_reg_bits((uintptr_t)&tpm_regs->status,
				TPM_STAT_VALID,
				chip_data->timeout_msec_c,
				&status);
	if ((ret < 0) || (status & TPM_STAT_EXPECT))
		return -1;

	ret = tpm2_fifo_io((uintptr_t)&tpm_regs->status,
			TPM_WRITE, BYTE, TPM_STAT_GO);
	if (ret < 0)
		return -1;

	return index;
}

int tpm2_fifo_receive(chip_data_t *chip_data, uint8_t *buf, size_t count)
{

	int size = 0, bytes_expected;
	uint8_t status;
	const uint16_t *length_offset = (uint16_t *)(buf+2);
	uint16_t burstcnt;
	tpm_registers *tpm_regs =
			LOCALITY_START_ADDRESS(chip_data, chip_data->locality);

	if (count < TPM_HEADER_SIZE)
		return -1;

	int ret = tpm2_wait_reg_bits((uintptr_t)&tpm_regs->status,
				TPM_STAT_AVAIL,
				chip_data->timeout_msec_c,
				&status);
	if (ret < 0)
		return -1;


	while (size < TPM_HEADER_SIZE) {
		burstcnt = tpm2_fifo_read_burstcount(chip_data);
		if (burstcnt < 0)
			return -1;

		for (; burstcnt > 0 ; burstcnt--, size++)
			*(buf + size) = tpm2_fifo_io((uintptr_t)&tpm_regs->data_fifo,
							TPM_READ, BYTE, 0);
	}

	bytes_expected = bswap32(*length_offset | *(length_offset + 1) << 16);
	if (bytes_expected > count) {
		ERROR("TPM : Command Buffer cannot store expected response\n");
		return -1;
	}

	if (size == bytes_expected) {
		return size;
	}

	ret = tpm2_wait_reg_bits((uintptr_t)&tpm_regs->status,
				TPM_STAT_AVAIL,
				chip_data->timeout_msec_c,
				&status);
	if (ret < 0)
		return -1;

	while (size < bytes_expected) {
		burstcnt = tpm2_fifo_read_burstcount(chip_data);
		if (burstcnt < 0)
			return -1;

		for (; burstcnt > 0 ; burstcnt--, size++)
			*(buf + size) = tpm2_fifo_io((uintptr_t)&tpm_regs->data_fifo,
							TPM_READ, BYTE, 0);
	}

	if (size < bytes_expected) {
		ERROR("TPM : Expected data not received\n");
		return -1;
	}

	return size;
}

static interface_ops_t fifo_ops = {
	.get_info = tpm2_get_info,
	.send = tpm2_fifo_send,
	.receive = tpm2_fifo_receive,
	.request_access = tpm2_fifo_request_access,
	.release_locality = tpm2_fifo_release_locality,
};

struct interface_ops *
tpm_interface_getops(chip_data_t *chip_data,  uint8_t locality)
{
	return &fifo_ops;
}
