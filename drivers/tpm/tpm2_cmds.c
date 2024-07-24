/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/tpm/tpm2.h>
#include <drivers/tpm/tpm2_chip.h>
#include <drivers/delay_timer.h>
#include <drivers/tpm/tpm2_interface.h>

#define CMD_SIZE_OFFSET 6

#define TWO_BYTES 2
#define FOUR_BYTES 4

struct interface_ops *interface;

int tpm_xfer(chip_data_t *chip_data, const uint8_t *send, uint8_t *receive)
{
	int ret;

	ret = interface->send(chip_data, (uint8_t *) send);
	if (ret < 0) {
		return ret;
	}

	ret = interface->receive(chip_data, receive, MAX_SIZE_CMDBUF);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int tpm_interface_init(chip_data_t *chip_data, uint8_t locality)
{
	int ret;

	interface = tpm_interface_getops(chip_data, locality);

	ret = interface->request_access(chip_data, locality);
	if (ret)
		return ret;

	interface->get_info(chip_data, locality);

	return 0;
}

void tpm_interface_close(chip_data_t *chip_data, uint8_t locality)
{
	interface->release_locality(chip_data, locality);
}

void tpm_update_buffer(tpm_cmd* buf, uint32_t new_data, size_t new_len)
{
	int i, j;
	int start = buf->data_length;

	if (new_len == FOUR_BYTES)
		new_data = bswap32(new_data);
	else if (new_len == TWO_BYTES)
		new_data = bswap16(new_data & 0xFFFF);

	uint8_t *ptr = (uint8_t *)&new_data;

	for (i = start, j = 0; i < start + new_len; i++, j++)
		buf->data[i] = *((ptr + j));
	buf->data_length = buf->data_length + new_len;
}


uint32_t tpm_startup(chip_data_t *chip_data, uint16_t mode)
{
	tpm_cmd startup_cmd;
	uint8_t response_buffer[MAX_SIZE_CMDBUF];
	int err, ret;

	memset(&startup_cmd, 0, sizeof(startup_cmd));

	startup_cmd.cmdinf.session = bswap16(TPM_ST_NO_SESSIONS);
	startup_cmd.cmdinf.cmd = bswap32(TPM_CMD_STARTUP);
	startup_cmd.data_length = 0;

	tpm_update_buffer(&startup_cmd, mode, sizeof(mode));

	startup_cmd.cmdinf.cmd_size = bswap32(sizeof(tpm_cmd_hdr) + startup_cmd.data_length);
	err = tpm_xfer(chip_data,
			(uint8_t *) &startup_cmd,
			(void *)&response_buffer);
	if (err < 0)
		return err;

	ret = bswap32(response_buffer[CMD_SIZE_OFFSET]);
	if(ret != TPM_SUCCESS)
		return -1;

	return 0;
}

uint32_t tpm_pcr_extend(chip_data_t *chip_data, uint32_t index,
		uint16_t algorithm, const uint8_t *digest,
		uint32_t digest_len)
{
	tpm_cmd pcr_extend_cmd;
	uint8_t response_buffer[MAX_SIZE_CMDBUF];
	int err, ret;

	memset(&pcr_extend_cmd, 0, sizeof(pcr_extend_cmd));

	if (!digest)
		return -EINVAL;
	pcr_extend_cmd.cmdinf.session = bswap16(TPM_ST_SESSIONS);
	pcr_extend_cmd.cmdinf.cmd = bswap32(TPM_CMD_PCR_EXTEND);
	pcr_extend_cmd.data_length = 0;

	/* handle (PCR Index)*/
	tpm_update_buffer(&pcr_extend_cmd, index, sizeof(index));
	/* authorization size , session handle, nonce size, attributes*/
	tpm_update_buffer(&pcr_extend_cmd, 9 , sizeof(uint32_t));
	tpm_update_buffer(&pcr_extend_cmd, TPM_RS_PW, sizeof(uint32_t));
	tpm_update_buffer(&pcr_extend_cmd, 0, sizeof(uint16_t));
	tpm_update_buffer(&pcr_extend_cmd, 0, 1);
	/* hmac/password size */
	tpm_update_buffer(&pcr_extend_cmd, 0,sizeof(uint16_t));
	/* hashes count */
	tpm_update_buffer(&pcr_extend_cmd, 1, sizeof(uint32_t));
	/* hash algorithm */
	tpm_update_buffer(&pcr_extend_cmd, algorithm, sizeof(algorithm));
	/* digest */
	memcpy(&pcr_extend_cmd.data[pcr_extend_cmd.data_length], digest, digest_len);

	pcr_extend_cmd.data_length += digest_len;
	pcr_extend_cmd.cmdinf.cmd_size += bswap32(sizeof(tpm_cmd_hdr) + pcr_extend_cmd.data_length);
	err = tpm_xfer(chip_data,
			(uint8_t *) &pcr_extend_cmd,
			(void *)&response_buffer);
	if(err < 0)
		return err;

	ret = bswap32(response_buffer[CMD_SIZE_OFFSET]);
	if(ret != TPM_SUCCESS)
		return -1;

	return 0;
}
