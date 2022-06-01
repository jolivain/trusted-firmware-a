/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* C lib */

/* TF-A header */
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/smccc.h>

/* MTK header */

/* GZ header */
#include "gz_secio.h"
#include "mtk_hyp_secio.h"

#define SECIO_DEBUG_ENABLE (0)
/* clang-format off */
#if (1 == SECIO_DEBUG_ENABLE)
#define SECIO_DBG(fmt, ...) NOTICE(fmt, ##__VA_ARGS__)
#else
#define SECIO_DBG(fmt, ...) do {} while (0)
#endif
/* clang-format on */

static const struct secio_register *secio_regs;

#define IO_OFFSET_LIMIT (0x1000)
#define IO_ACCESS_SIZE (4)
static inline bool is_valid_offset(uint32_t io_type, uint32_t offset)
{
	uint32_t io_size;

	secio_regs = get_plat_secio_regs();
	io_size = secio_regs[io_type].base_size;

	/* size 4k align */
	if ((io_size % IO_OFFSET_LIMIT) != 0)
		return false;

	/* over range */
	if (offset >= io_size)
		return false;

	/* valid register offset */
	if ((offset % IO_ACCESS_SIZE) != 0)
		return false;

	return true;
}

static inline bool is_valid_secio_type(uint32_t type)
{
	if ((type > SECIO_INVALID) && (type < SECIO_MAX))
		return true;
	return false;
}

#define SECIO_ERR (-1)
uint64_t sip_secio_write(u_register_t x1, u_register_t x2, u_register_t x3,
			 u_register_t x4, void *handle)
{
	uint32_t io_type = (uint32_t)x1;
	uint32_t reg_offset = (uint32_t)x2;
	uint32_t reg_val = (uint32_t)x3;
	uintptr_t reg_addr;

	if (!is_valid_secio_type(io_type)) {
		ERROR("SECIO: invalid write io type=%d!\n", io_type);
		return SECIO_ERR;
	}

	if (!is_valid_offset(io_type, reg_offset)) {
		ERROR("SECIO: invalid write io offset=0x%x!\n", reg_offset);
		return SECIO_ERR;
	}

	secio_regs = get_plat_secio_regs();
	reg_addr = secio_regs[io_type].base_addr;
	if (secio_regs[io_type].base_addr == SECIO_INVALID) {
		ERROR("SECIO: invalid base addr=0x%x, io_type=%d!\n",
		      (uint32_t)secio_regs[io_type].base_addr, io_type);
		return SECIO_ERR;
	}

	reg_addr += reg_offset;
	mmio_write_32(reg_addr, reg_val);

	SECIO_DBG("SECIO: w_addr=0x%x, val=0x%x, after=0x%x\n",
		  (uint32_t)reg_addr, reg_val, mmio_read_32(reg_addr));
	return SMC_OK;
}

uint64_t sip_secio_read(u_register_t x1, u_register_t x2, u_register_t x3,
			u_register_t x4, void *handle, uint32_t *rc_1)
{
	uint32_t io_type = (uint32_t)x1;
	uint32_t reg_offset = (uint32_t)x2;
	uint32_t reg_val;
	uintptr_t reg_addr;

	if (!is_valid_secio_type(io_type)) {
		ERROR("SECIO: invalid read io type=%d!\n", io_type);
		return SECIO_ERR;
	}

	if (!is_valid_offset(io_type, reg_offset)) {
		ERROR("SECIO: invalid write io offset=0x%x!\n", reg_offset);
		return SECIO_ERR;
	}

	secio_regs = get_plat_secio_regs();
	reg_addr = secio_regs[io_type].base_addr;
	if (secio_regs[io_type].base_addr == SECIO_INVALID) {
		ERROR("SECIO: invalid base addr=0x%x, io_type=%d!\n",
		      (uint32_t)secio_regs[io_type].base_addr, io_type);
		return SECIO_ERR;
	}

	reg_addr += reg_offset;
	reg_val = mmio_read_32(reg_addr);

	if (rc_1)
		*rc_1 = reg_val;

	SECIO_DBG("SECIO: r_addr=0x%x, val=0x%x\n", (uint32_t)reg_addr,
		  reg_val);
	return SMC_OK;
}
