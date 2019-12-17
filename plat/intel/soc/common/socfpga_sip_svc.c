/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <tools_share/uuid.h>

#include "socfpga_mailbox.h"

/* Number of SiP Calls implemented */
#define SIP_NUM_CALLS		0x3

/* Total buffer the driver can hold */
#define FPGA_CONFIG_BUFFER_SIZE 4

int current_block;
int current_buffer;
int current_id = 1;
int max_blocks;
uint32_t bytes_per_block;
uint32_t blocks_submitted;
uint32_t blocks_completed;

struct fpga_config_info {
	uint32_t addr;
	int size;
	int size_written;
	uint32_t write_requested;
	int subblocks_sent;
	int block_number;
};

/*  SiP Service UUID */
DEFINE_SVC_UUID2(intl_svc_uid,
		0xa85273b0, 0xe85a, 0x4862, 0xa6, 0x2a,
		0xfa, 0x88, 0x88, 0x17, 0x68, 0x81);

uint64_t socfpga_sip_handler(uint32_t smc_fid,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

struct fpga_config_info fpga_config_buffers[FPGA_CONFIG_BUFFER_SIZE];

static void intel_fpga_sdm_write_buffer(struct fpga_config_info *buffer)
{
	uint32_t args[3];

	while (max_blocks > 0 && buffer->size > buffer->size_written) {
		if (buffer->size - buffer->size_written <=
			bytes_per_block) {
			args[0] = (1<<8);
			args[1] = buffer->addr + buffer->size_written;
			args[2] = buffer->size - buffer->size_written;
			buffer->size_written +=
				buffer->size - buffer->size_written;
			buffer->subblocks_sent++;
			mailbox_send_cmd_async(0x4,
				MBOX_RECONFIG_DATA,
				args, 3, 0);
			current_buffer++;
			current_buffer %= FPGA_CONFIG_BUFFER_SIZE;
		} else {
			args[0] = (1<<8);
			args[1] = buffer->addr + buffer->size_written;
			args[2] = bytes_per_block;
			buffer->size_written += bytes_per_block;
			mailbox_send_cmd_async(0x4,
				MBOX_RECONFIG_DATA,
				args, 3, 0);
			buffer->subblocks_sent++;
		}
		max_blocks--;
	}
}

static int intel_fpga_sdm_write_all(void)
{
	int i;

	for (i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++)
		intel_fpga_sdm_write_buffer(
			&fpga_config_buffers[current_buffer]);

	return 0;
}

uint32_t intel_mailbox_fpga_config_isdone(void)
{
	uint32_t args[2];
	uint32_t response[6];
	int status;

	status = mailbox_send_cmd(1, MBOX_RECONFIG_STATUS, args, 0, 0,
				response);

	if (status < 0)
		return INTEL_SIP_SMC_STATUS_ERROR;

	if (response[RECONFIG_STATUS_STATE] &&
		response[RECONFIG_STATUS_STATE] != MBOX_CFGSTAT_STATE_CONFIG)
		return INTEL_SIP_SMC_STATUS_ERROR;

	if (!(response[RECONFIG_STATUS_PIN_STATUS] & PIN_STATUS_NSTATUS))
		return INTEL_SIP_SMC_STATUS_ERROR;

	if (response[RECONFIG_STATUS_SOFTFUNC_STATUS] &
		SOFTFUNC_STATUS_SEU_ERROR)
		return INTEL_SIP_SMC_STATUS_ERROR;

	if ((response[RECONFIG_STATUS_SOFTFUNC_STATUS] &
		SOFTFUNC_STATUS_CONF_DONE) &&
		(response[RECONFIG_STATUS_SOFTFUNC_STATUS] &
		SOFTFUNC_STATUS_INIT_DONE))
		return INTEL_SIP_SMC_STATUS_OK;

	return INTEL_SIP_SMC_STATUS_ERROR;
}

static int mark_last_buffer_xfer_completed(uint32_t *buffer_addr_completed)
{
	int i;

	for (i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		if (fpga_config_buffers[i].block_number == current_block) {
			fpga_config_buffers[i].subblocks_sent--;
			if (fpga_config_buffers[i].subblocks_sent == 0
			&& fpga_config_buffers[i].size <=
			fpga_config_buffers[i].size_written) {
				fpga_config_buffers[i].write_requested = 0;
				current_block++;
				*buffer_addr_completed =
					fpga_config_buffers[i].addr;
				return 0;
			}
		}
	}

	return -1;
}

unsigned int address_in_ddr(uint32_t *addr)
{
	if (((unsigned long long)addr > DRAM_BASE) &&
		((unsigned long long)addr < DRAM_BASE + DRAM_SIZE))
		return 0;

	return -1;
}

int intel_fpga_config_completed_write(uint32_t *completed_addr,
					uint32_t *count)
{
	uint32_t status = INTEL_SIP_SMC_STATUS_OK;
	*count = 0;
	int resp_len = 0;
	uint32_t resp[5];
	int all_completed = 1;
	int count_check = 0;

	if (address_in_ddr(completed_addr) != 0 || address_in_ddr(count) != 0)
		return INTEL_SIP_SMC_STATUS_ERROR;

	for (count_check = 0; count_check < 3; count_check++)
		if (address_in_ddr(&completed_addr[*count + count_check]) != 0)
			return INTEL_SIP_SMC_STATUS_ERROR;

	resp_len = mailbox_read_response(0x4, resp);

	while (resp_len >= 0 && *count < 3) {
		max_blocks++;
		if (mark_last_buffer_xfer_completed(
			&completed_addr[*count]) == 0)
			*count = *count + 1;
		else
			break;
		resp_len = mailbox_read_response(0x4, resp);
	}

	if (*count <= 0) {
		if (resp_len != MBOX_NO_RESPONSE &&
			resp_len != MBOX_TIMEOUT && resp_len != 0) {
			return INTEL_SIP_SMC_STATUS_ERROR;
		}

		*count = 0;
	}

	intel_fpga_sdm_write_all();

	if (*count > 0)
		status = INTEL_SIP_SMC_STATUS_OK;
	else if (*count == 0)
		status = INTEL_SIP_SMC_STATUS_BUSY;

	for (int i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		if (fpga_config_buffers[i].write_requested != 0) {
			all_completed = 0;
			break;
		}
	}

	if (all_completed == 1)
		return INTEL_SIP_SMC_STATUS_OK;

	return status;
}

int intel_fpga_config_start(uint32_t config_type)
{
	uint32_t response[3];
	int status = 0;

	status = mailbox_send_cmd(2, MBOX_RECONFIG, 0, 0, 0,
			response);

	if (status < 0)
		return status;

	max_blocks = response[0];
	bytes_per_block = response[1];

	for (int i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		fpga_config_buffers[i].size = 0;
		fpga_config_buffers[i].size_written = 0;
		fpga_config_buffers[i].addr = 0;
		fpga_config_buffers[i].write_requested = 0;
		fpga_config_buffers[i].block_number = 0;
		fpga_config_buffers[i].subblocks_sent = 0;
	}

	blocks_submitted = 0;
	current_block = 0;
	current_buffer = 0;

	return 0;
}


uint32_t intel_fpga_config_write(uint64_t mem, uint64_t size)
{
	int i = 0;
	uint32_t status = INTEL_SIP_SMC_STATUS_OK;

	if (mem < DRAM_BASE || mem > DRAM_BASE + DRAM_SIZE)
		status = INTEL_SIP_SMC_STATUS_REJECTED;

	if (mem + size > DRAM_BASE + DRAM_SIZE)
		status = INTEL_SIP_SMC_STATUS_REJECTED;

	for (i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		if (!fpga_config_buffers[i].write_requested) {
			fpga_config_buffers[i].addr = mem;
			fpga_config_buffers[i].size = size;
			fpga_config_buffers[i].size_written = 0;
			fpga_config_buffers[i].write_requested = 1;
			fpga_config_buffers[i].block_number =
				blocks_submitted++;
			fpga_config_buffers[i].subblocks_sent = 0;
			break;
		}
	}


	if (i == FPGA_CONFIG_BUFFER_SIZE) {
		status = INTEL_SIP_SMC_STATUS_REJECTED;
		return status;
	} else if (i == FPGA_CONFIG_BUFFER_SIZE - 1) {
		status = INTEL_SIP_SMC_STATUS_BUSY;
	}

	intel_fpga_sdm_write_all();

	return status;
}

static int is_out_of_sec_range(uint64_t reg_addr)
{
	switch (reg_addr) {
	case(0xF8011100):	/* ECCCTRL1 */
	case(0xF8011104):	/* ECCCTRL2 */
	case(0xF8011110):	/* ERRINTEN */
	case(0xF8011114):	/* ERRINTENS */
	case(0xF8011118):	/* ERRINTENR */
	case(0xF801111C):	/* INTMODE */
	case(0xF8011120):	/* INTSTAT */
	case(0xF8011124):	/* DIAGINTTEST */
	case(0xF801112C):	/* DERRADDRA */
	case(0xFFD12028):	/* SDMMCGRP_CTRL */
	case(0xFFD12044):	/* EMAC0 */
	case(0xFFD12048):	/* EMAC1 */
	case(0xFFD1204C):	/* EMAC2 */
	case(0xFFD12090):	/* ECC_INT_MASK_VALUE */
	case(0xFFD12094):	/* ECC_INT_MASK_SET */
	case(0xFFD12098):	/* ECC_INT_MASK_CLEAR */
	case(0xFFD1209C):	/* ECC_INTSTATUS_SERR */
	case(0xFFD120A0):	/* ECC_INTSTATUS_DERR */
	case(0xFFD120C0):	/* NOC_TIMEOUT */
	case(0xFFD120C4):	/* NOC_IDLEREQ_SET */
	case(0xFFD120C8):	/* NOC_IDLEREQ_CLR */
	case(0xFFD120D0):	/* NOC_IDLEACK */
	case(0xFFD120D4):	/* NOC_IDLESTATUS */
	case(0xFFD12200):	/* BOOT_SCRATCH_COLD0 */
	case(0xFFD12204):	/* BOOT_SCRATCH_COLD1 */
	case(0xFFD12220):	/* BOOT_SCRATCH_COLD8 */
	case(0xFFD12224):	/* BOOT_SCRATCH_COLD9 */
		return 0;

	default:
		break;
	}

	return -1;
}

/* Secure register access */
uint32_t intel_secure_reg_read(uint64_t reg_addr, uint32_t *retval)
{
	if (is_out_of_sec_range(reg_addr))
		return INTEL_SIP_SMC_STATUS_ERROR;

	*retval = mmio_read_32(reg_addr);

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_secure_reg_write(uint64_t reg_addr, uint32_t val,
				uint32_t *retval)
{
	if (is_out_of_sec_range(reg_addr))
		return INTEL_SIP_SMC_STATUS_ERROR;

	mmio_write_32(reg_addr, val);

	return intel_secure_reg_read(reg_addr, retval);
}

uint32_t intel_secure_reg_update(uint64_t reg_addr, uint32_t mask,
				 uint32_t val, uint32_t *retval)
{
	if (!intel_secure_reg_read(reg_addr, retval)) {
		*retval &= ~mask;
		*retval |= val;
		return intel_secure_reg_write(reg_addr, *retval, retval);
	}

	return INTEL_SIP_SMC_STATUS_ERROR;
}

/*
 * This function is responsible for handling all SiP calls from the NS world
 */

uintptr_t sip_smc_handler(uint32_t smc_fid,
			 u_register_t x1,
			 u_register_t x2,
			 u_register_t x3,
			 u_register_t x4,
			 void *cookie,
			 void *handle,
			 u_register_t flags)
{
	uint32_t val = 0;
	uint32_t status = INTEL_SIP_SMC_STATUS_OK;
	uint32_t completed_addr[3];
	uint32_t count = 0;

	switch (smc_fid) {
	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, intl_svc_uid);

	case INTEL_SIP_SMC_FPGA_CONFIG_ISDONE:
		status = intel_mailbox_fpga_config_isdone();
		SMC_RET4(handle, status, 0, 0, 0);

	case INTEL_SIP_SMC_FPGA_CONFIG_GET_MEM:
		SMC_RET3(handle, INTEL_SIP_SMC_STATUS_OK,
			INTEL_SIP_SMC_FPGA_CONFIG_ADDR,
			INTEL_SIP_SMC_FPGA_CONFIG_SIZE -
				INTEL_SIP_SMC_FPGA_CONFIG_ADDR);

	case INTEL_SIP_SMC_FPGA_CONFIG_START:
		status = intel_fpga_config_start(x1);
		SMC_RET4(handle, status, 0, 0, 0);

	case INTEL_SIP_SMC_FPGA_CONFIG_WRITE:
		status = intel_fpga_config_write(x1, x2);
		SMC_RET4(handle, status, 0, 0, 0);

	case INTEL_SIP_SMC_FPGA_CONFIG_COMPLETED_WRITE:
		status = intel_fpga_config_completed_write(completed_addr,
								&count);
		switch (count) {
		case 1:
			SMC_RET4(handle, INTEL_SIP_SMC_STATUS_OK,
				completed_addr[0], 0, 0);

		case 2:
			SMC_RET4(handle, INTEL_SIP_SMC_STATUS_OK,
				completed_addr[0],
				completed_addr[1], 0);

		case 3:
			SMC_RET4(handle, INTEL_SIP_SMC_STATUS_OK,
				completed_addr[0],
				completed_addr[1],
				completed_addr[2]);

		case 0:
			SMC_RET4(handle, status, 0, 0, 0);

		default:
			SMC_RET1(handle, INTEL_SIP_SMC_STATUS_ERROR);
		}

	case INTEL_SIP_SMC_REG_READ:
		status = intel_secure_reg_read(x1, &val);
		SMC_RET3(handle, status, val, x1);

	case INTEL_SIP_SMC_REG_WRITE:
		status = intel_secure_reg_write(x1, (uint32_t)x2, &val);
		SMC_RET3(handle, status, val, x1);

	case INTEL_SIP_SMC_REG_UPDATE:
		status = intel_secure_reg_update(x1, (uint32_t)x2,
						 (uint32_t)x3, &val);
		SMC_RET3(handle, status, val, x1);

	default:
		return socfpga_sip_handler(smc_fid, x1, x2, x3, x4,
			cookie, handle, flags);
	}
}

DECLARE_RT_SVC(
	socfpga_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	sip_smc_handler
);

DECLARE_RT_SVC(
	socfpga_sip_svc_std,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_YIELD,
	NULL,
	sip_smc_handler
);
