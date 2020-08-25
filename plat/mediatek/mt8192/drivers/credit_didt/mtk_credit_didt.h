/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_CREDIT_DIDT_H
#define MTK_CREDIT_DIDT_H

#include <common/debug.h>
#include <lib/mmio.h>

/************************************************
 * BIT Operation and REG r/w
 ************************************************/
#undef  BIT
#define BIT(_bit_)                    (unsigned int)(1 << (_bit_))
#define BITS(_bits_, _val_)           ((((unsigned int) -1 >> (31 - ((1) ? _bits_))) \
& ~((1U << ((0) ? _bits_)) - 1)) & ((_val_)<<((0) ? _bits_)))
#define BITMASK(_bits_)               (((unsigned int) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1))
#define GET_BITS_VAL(_bits_, _val_)   (((_val_) & (BITMASK(_bits_))) >> ((0) ? _bits_))

#define credit_didt_read(addr)			mmio_read_32((uintptr_t)addr)
#define credit_didt_write(addr,  val)	mmio_write_32((uintptr_t)addr,  val)

#define credit_didt_shift(val, bits, shift) \
	((val & (((unsigned int)0x1 << bits) - 1)) << shift)

#define credit_didt_write_bits(addr, bits, shift, val) \
	credit_didt_write(addr, ((credit_didt_read(addr) & ~(credit_didt_shift((unsigned int)0xffffffff, \
	bits, shift))) | credit_didt_shift(val, bits, shift)))

#define credit_didt_read_bits(addr, bits, shift) \
	((credit_didt_read(addr) >> shift) & (((unsigned int)0x1 << bits) - 1))

#define credit_didt_set_bit(addr, set) credit_didt_write(addr, (credit_didt_read(addr) | (set)))
#define credit_didt_clr_bit(addr, clr) credit_didt_write(addr, (credit_didt_read(addr) & ~(clr)))

/************************************************
 * Need maintain for each project
 ************************************************/
/* Core ID control by project */
#define CREDIT_PRJ_CFG

#ifdef CREDIT_PRJ_CFG
#define NR_CREDIT_DIDT_CPU 4
#define CREDIT_DIDT_CPU_START_ID 4
#define CREDIT_DIDT_CPU_END_ID 7
#endif

/************************************************
 * Register addr, offset, bits range
 ************************************************/

/* CREDIT_DIDT AO control register */
#define CREDIT_DIDT_MCUSYS_REG_BASE_ADDR			(0x0C530000)
#define CREDIT_DIDT_CPU4_DIDT_REG					(CREDIT_DIDT_MCUSYS_REG_BASE_ADDR + 0xB830)
#define CREDIT_DIDT_CPU5_DIDT_REG					(CREDIT_DIDT_MCUSYS_REG_BASE_ADDR + 0xBA30)
#define CREDIT_DIDT_CPU6_DIDT_REG					(CREDIT_DIDT_MCUSYS_REG_BASE_ADDR + 0xBC30)
#define CREDIT_DIDT_CPU7_DIDT_REG					(CREDIT_DIDT_MCUSYS_REG_BASE_ADDR + 0xBE30)

static const unsigned int CREDIT_DIDT_CPU_AO_BASE[NR_CREDIT_DIDT_CPU] = {
	CREDIT_DIDT_CPU4_DIDT_REG,
	CREDIT_DIDT_CPU5_DIDT_REG,
	CREDIT_DIDT_CPU6_DIDT_REG,
	CREDIT_DIDT_CPU7_DIDT_REG
};

#define CREDIT_DIDT_BITS_LS_CTRL_EN		1
#define CREDIT_DIDT_BITS_LS_INDEX_SEL	1
#define CREDIT_DIDT_BITS_VX_CTRL_EN		1

#define CREDIT_DIDT_SHIFT_LS_CTRL_EN	15
#define CREDIT_DIDT_SHIFT_LS_INDEX_SEL	17
#define CREDIT_DIDT_SHIFT_VX_CTRL_EN	18

/* CREDIT_DIDT control register */
#define CREDIT_DIDT_CPU4_CONFIG_REG	(0x0C532000)
#define CREDIT_DIDT_CPU5_CONFIG_REG	(0x0C532800)
#define CREDIT_DIDT_CPU6_CONFIG_REG	(0x0C533000)
#define CREDIT_DIDT_CPU7_CONFIG_REG	(0x0C533800)

#ifdef CREDIT_PRJ_CFG
static const unsigned int CREDIT_DIDT_CPU_BASE[NR_CREDIT_DIDT_CPU] = {
	CREDIT_DIDT_CPU4_CONFIG_REG,
	CREDIT_DIDT_CPU5_CONFIG_REG,
	CREDIT_DIDT_CPU6_CONFIG_REG,
	CREDIT_DIDT_CPU7_CONFIG_REG
};
#endif

#define CREDIT_DIDT_DIDT_CONTROL		(0x480)

#define CREDIT_DIDT_BITS_LS_CFG_CREDIT			5
#define CREDIT_DIDT_BITS_LS_CFG_PERIOD			3
#define CREDIT_DIDT_BITS_LS_CFG_LOW_PERIOD		3
#define CREDIT_DIDT_BITS_LS_CFG_LOW_FREQ_EN		1
#define CREDIT_DIDT_BITS_VX_CFG_CREDIT			5
#define CREDIT_DIDT_BITS_VX_CFG_PERIOD			3
#define CREDIT_DIDT_BITS_VX_CFG_LOW_PERIOD		3
#define CREDIT_DIDT_BITS_VX_CFG_LOW_FREQ_EN		1
#define CREDIT_DIDT_BITS_CONST_MODE				1

#define CREDIT_DIDT_SHIFT_LS_CFG_CREDIT			0
#define CREDIT_DIDT_SHIFT_LS_CFG_PERIOD			5
#define CREDIT_DIDT_SHIFT_LS_CFG_LOW_PERIOD		8
#define CREDIT_DIDT_SHIFT_LS_CFG_LOW_FREQ_EN	11
#define CREDIT_DIDT_SHIFT_VX_CFG_CREDIT			16
#define CREDIT_DIDT_SHIFT_VX_CFG_PERIOD			21
#define CREDIT_DIDT_SHIFT_VX_CFG_LOW_PERIOD		24
#define CREDIT_DIDT_SHIFT_VX_CFG_LOW_FREQ_EN	27
#define CREDIT_DIDT_SHIFT_CONST_MODE			31

/************************************************
 * config enum
 ************************************************/
enum CREDIT_DIDT_RW {
	CREDIT_DIDT_RW_READ,
	CREDIT_DIDT_RW_WRITE,
	CREDIT_DIDT_RW_REG_READ,
	CREDIT_DIDT_RW_REG_WRITE,

	NR_CREDIT_DIDT_RW,
};

enum CREDIT_DIDT_CHANNEL {
	CREDIT_DIDT_CHANNEL_LS,
	CREDIT_DIDT_CHANNEL_VX,

	NR_CREDIT_DIDT_CHANNEL,
};

enum CREDIT_DIDT_CFG {
	CREDIT_DIDT_CFG_PERIOD,
	CREDIT_DIDT_CFG_CREDIT,
	CREDIT_DIDT_CFG_LOW_PWR_PERIOD,
	CREDIT_DIDT_CFG_LOW_PWR_ENABLE,
	CREDIT_DIDT_CFG_ENABLE,

	NR_CREDIT_DIDT_CFG,
};

enum CREDIT_DIDT_PARAM {

	CREDIT_DIDT_PARAM_LS_PERIOD =
		CREDIT_DIDT_CHANNEL_LS * NR_CREDIT_DIDT_CFG,
	CREDIT_DIDT_PARAM_LS_CREDIT,
	CREDIT_DIDT_PARAM_LS_LOW_PWR_PERIOD,
	CREDIT_DIDT_PARAM_LS_LOW_PWR_ENABLE,
	CREDIT_DIDT_PARAM_LS_ENABLE,

	CREDIT_DIDT_PARAM_VX_PERIOD =
		CREDIT_DIDT_CHANNEL_VX * NR_CREDIT_DIDT_CFG,
	CREDIT_DIDT_PARAM_VX_CREDIT,
	CREDIT_DIDT_PARAM_VX_LOW_PWR_PERIOD,
	CREDIT_DIDT_PARAM_VX_LOW_PWR_ENABLE,
	CREDIT_DIDT_PARAM_VX_ENABLE,

	CREDIT_DIDT_PARAM_CONST_MODE,
	CREDIT_DIDT_PARAM_LS_IDX_SEL,

	NR_CREDIT_DIDT_PARAM,
};


/************************************************
 * association with ATF use
 ************************************************/
extern int credit_didt_init(void);
extern int credit_didt_control(int a0, int a1, int a2, int a3);

#endif /* end of MTK_CREDIT_DIDT_H */

