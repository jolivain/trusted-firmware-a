/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_BRISKET_H
#define MTK_BRISKET_H

#include <common/debug.h>
#include <lib/mmio.h>

#define SYSAPB_BASE                          (0x40000000)

/************************************************
 * BIT Operation and REG r/w
 ************************************************/
#undef  BIT
#define BIT(_bit_)                    (unsigned int)(1 << (_bit_))
#define BITS(_bits_, _val_)           ((((unsigned int) -1 >> (31 - ((1) ? _bits_))) \
& ~((1U << ((0) ? _bits_)) - 1)) & ((_val_)<<((0) ? _bits_)))
#define BITMASK(_bits_)               (((unsigned int) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1))
#define GET_BITS_VAL(_bits_, _val_)   (((_val_) & (BITMASK(_bits_))) >> ((0) ? _bits_))

#define brisket_read(addr)			mmio_read_32((uintptr_t)addr)
#define brisket_write(addr,  val)	mmio_write_32((uintptr_t)addr,  val)

#define brisket_shift(val, bits, shift) \
	((val & (((unsigned int)0x1 << bits) - 1)) << shift)

#define brisket_write_bits(addr, bits, shift, val) \
	brisket_write(addr, ((brisket_read(addr) & ~(brisket_shift((unsigned int)0xffffffff, \
	bits, shift))) | brisket_shift(val, bits, shift)))

#define brisket_read_bits(addr, bits, shift) \
	((brisket_read(addr) >> shift) & (((unsigned int)0x1 << bits) - 1))

#define brisket_set_bit(addr, set) brisket_write(addr, (brisket_read(addr) | (set)))
#define brisket_clr_bit(addr, clr) brisket_write(addr, (brisket_read(addr) & ~(clr)))

/************************************************
 * Need maintain for each project
 ************************************************/
/* Core ID control by project */
#define BRISKET_PRJ_CFG

#ifdef BRISKET_PRJ_CFG
#define NR_BRISKET_CPU 8
#define BRISKET_CPU_START_ID 0
#define BRISKET_CPU_END_ID 7
#endif

/************************************************
 * Register addr, offset, bits range
 ************************************************/
/* BRISKET base_addr */
#ifdef BRISKET_PRJ_CFG
#define BRISKET_CPU0_CONFIG_REG	(0x0C530000)
#define BRISKET_CPU1_CONFIG_REG	(0x0C530800)
#define BRISKET_CPU2_CONFIG_REG	(0x0C531000)
#define BRISKET_CPU3_CONFIG_REG	(0x0C531800)
#define BRISKET_CPU4_CONFIG_REG	(0x0C532000)
#define BRISKET_CPU5_CONFIG_REG	(0x0C532800)
#define BRISKET_CPU6_CONFIG_REG	(0x0C533000)
#define BRISKET_CPU7_CONFIG_REG	(0x0C533800)
#endif

/* BRISKET control register offset */
#define BRISKET_CONTROL_OFFSET	(0x310)
#define BRISKET01_OFFSET		(0x600)
#define BRISKET02_OFFSET		(0x604)
#define BRISKET03_OFFSET		(0x608)
#define BRISKET04_OFFSET		(0x60C)
#define BRISKET05_OFFSET		(0x610)
#define BRISKET06_OFFSET		(0x614)
#define BRISKET07_OFFSET		(0x618)
#define BRISKET08_OFFSET		(0x61C)
#define BRISKET09_OFFSET		(0x620)

/* BRISKET control register range */
/* BRISKET_CONTROL config */
#define BRISKET_CONTROL_BITS_Pllclken		1
/* BRISKET01 config */
#define BRISKET01_BITS_ErrOnline			11
#define BRISKET01_BITS_ErrOffline			11
#define BRISKET01_BITS_fsm_state			2
/* BRISKET02 config */
#define BRISKET02_BITS_cctrl_ping			6
#define BRISKET02_BITS_fctrl_ping			6
#define BRISKET02_BITS_cctrl_pong			6
#define BRISKET02_BITS_fctrl_pong			6
/* BRISKET03 config */
#define BRISKET03_BITS_InFreq				8
#define BRISKET03_BITS_OutFreq				8
#define BRISKET03_BITS_CalFreq				8
/* BRISKET04 config */
#define BRISKET04_BITS_Status				8
#define BRISKET04_BITS_PhaseErr				16
#define BRISKET04_BITS_LockDet				1
#define BRISKET04_BITS_Clk26mDet			1
/* BRISKET05 config */
#define BRISKET05_BITS_Bren					1
#define BRISKET05_BITS_KpOnline				4
#define BRISKET05_BITS_KiOnline				6
#define BRISKET05_BITS_KpOffline			4
#define BRISKET05_BITS_KiOffline			6
/* BRISKET06 config */
#define BRISKET06_BITS_FreqErrWtOnline		6
#define BRISKET06_BITS_FreqErrWtOffline		6
#define BRISKET06_BITS_PhaseErrWt			4
/* BRISKET07 config */
#define BRISKET07_BITS_FreqErrCapOnline		4
#define BRISKET07_BITS_FreqErrCapOffline	4
#define BRISKET07_BITS_PhaseErrCap			4
/* BRISKET08 config */
#define BRISKET08_BITS_PingMaxThreshold		6
#define BRISKET08_BITS_PongMinThreshold		6
#define BRISKET08_BITS_StartInPong			1
/* BRISKET09 config */
#define BRISKET09_BITS_PhlockThresh			3
#define BRISKET09_BITS_PhlockCycles			3
#define BRISKET09_BITS_Control				8

/* BRISKET_CONTROL config */
#define BRISKET_CONTROL_SHIFT_Pllclken		0
/* BRISKET01 config */
#define BRISKET01_SHIFT_ErrOnline			13
#define BRISKET01_SHIFT_ErrOffline			2
#define BRISKET01_SHIFT_fsm_state			0
/* BRISKET02 config */
#define BRISKET02_SHIFT_cctrl_ping			18
#define BRISKET02_SHIFT_fctrl_ping			12
#define BRISKET02_SHIFT_cctrl_pong			6
#define BRISKET02_SHIFT_fctrl_pong			0
/* BRISKET03 config */
#define BRISKET03_SHIFT_InFreq				16
#define BRISKET03_SHIFT_OutFreq				8
#define BRISKET03_SHIFT_CalFreq				0
/* BRISKET04 config */
#define BRISKET04_SHIFT_Status				18
#define BRISKET04_SHIFT_PhaseErr			2
#define BRISKET04_SHIFT_LockDet				1
#define BRISKET04_SHIFT_Clk26mDet			0
/* BRISKET05 config */
#define BRISKET05_SHIFT_Bren				20
#define BRISKET05_SHIFT_KpOnline			16
#define BRISKET05_SHIFT_KiOnline			10
#define BRISKET05_SHIFT_KpOffline			6
#define BRISKET05_SHIFT_KiOffline			0
/* BRISKET06 config */
#define BRISKET06_SHIFT_FreqErrWtOnline		10
#define BRISKET06_SHIFT_FreqErrWtOffline	4
#define BRISKET06_SHIFT_PhaseErrWt			0
/* BRISKET07 config */
#define BRISKET07_SHIFT_FreqErrCapOnline	8
#define BRISKET07_SHIFT_FreqErrCapOffline	4
#define BRISKET07_SHIFT_PhaseErrCap			0
/* BRISKET08 config */
#define BRISKET08_SHIFT_PingMaxThreshold	7
#define BRISKET08_SHIFT_PongMinThreshold	1
#define BRISKET08_SHIFT_StartInPong			0
/* BRISKET09 config */
#define BRISKET09_SHIFT_PhlockThresh		11
#define BRISKET09_SHIFT_PhlockCycles		8
#define BRISKET09_SHIFT_Control				0

#ifdef BRISKET_PRJ_CFG
static const unsigned int BRISKET_CPU_BASE[NR_BRISKET_CPU] = {
	BRISKET_CPU0_CONFIG_REG,
	BRISKET_CPU1_CONFIG_REG,
	BRISKET_CPU2_CONFIG_REG,
	BRISKET_CPU3_CONFIG_REG,
	BRISKET_CPU4_CONFIG_REG,
	BRISKET_CPU5_CONFIG_REG,
	BRISKET_CPU6_CONFIG_REG,
	BRISKET_CPU7_CONFIG_REG
};
#endif

#define BRISKET05_MASK_Bren 0xFFEFFFFF

/************************************************
 * config enum
 ************************************************/
enum BRISKET_RW {
	BRISKET_RW_READ,
	BRISKET_RW_WRITE,

	NR_BRISKET_RW,
};

enum BRISKET_GROUP {
	BRISKET_GROUP_CONTROL,
	BRISKET_GROUP_01,
	BRISKET_GROUP_02,
	BRISKET_GROUP_03,
	BRISKET_GROUP_04,
	BRISKET_GROUP_05,
	BRISKET_GROUP_06,
	BRISKET_GROUP_07,
	BRISKET_GROUP_08,
	BRISKET_GROUP_09,

	NR_BRISKET_GROUP,
};


/************************************/
extern int brisket_init(unsigned int en);
extern int brisket_control(unsigned int rw,
	unsigned int cpu, unsigned int brisket_group_bits_shift, unsigned int value);
/************************************/

#endif /* end of MTK_BRISKET_H */

