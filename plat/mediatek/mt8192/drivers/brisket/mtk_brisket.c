/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include "mtk_brisket.h"
#include <plat/common/platform.h>

/************************************************
 * Central control
 ************************************************/

/*If need enable Brisket@ATF, enable this cfg, else disable*/
#define BRISKET_EN_CTRL_ATF

/*If need debug msg for init setting, enable this cfg, else disable */
/* #define BRISKET_DEBUG */

/************************************************
 * Debug print
 ************************************************/
#define BRISKET_TAG	 "[BRISKET]"
#define brisket_err(fmt, args...)	ERROR(BRISKET_TAG"[ERROR][%s():%d]" fmt, __func__, __LINE__, ##args)
#define brisket_msg(fmt, args...)	INFO(BRISKET_TAG"[INFO][%s():%d]" fmt, __func__, __LINE__, ##args)

#ifdef BRISKET_DEBUG
#define brisket_debug(fmt, args...)	INFO(BRISKET_TAG"[DEBUG][%s():%d]" fmt, __func__, __LINE__, ##args)
#else
#define brisket_debug(fmt, args...)
#endif

#ifdef BRISKET_PRJ_CFG
static unsigned int brisket_info[NR_BRISKET_CPU][NR_BRISKET_GROUP] = {
	{1, 0, 0, 0, 0, 0x00110842, 0x0000fbf1, 0x00000fff, 0x00000c8a, 0x00003e00},
	{1, 0, 0, 0, 0, 0x00110842, 0x0000fbf1, 0x00000fff, 0x00000c8a, 0x00003e00},
	{1, 0, 0, 0, 0, 0x00110842, 0x0000fbf1, 0x00000fff, 0x00000c8a, 0x00003e00},
	{1, 0, 0, 0, 0, 0x00110842, 0x0000fbf1, 0x00000fff, 0x00000c8a, 0x00003e00},
	{1, 0, 0, 0, 0, 0x00110842, 0x0000fbf1, 0x00000fff, 0x00000c8a, 0x00003e00},
	{1, 0, 0, 0, 0, 0x00110842, 0x0000fbf1, 0x00000fff, 0x00000c8a, 0x00003e00},
	{1, 0, 0, 0, 0, 0x00110842, 0x0000fbf1, 0x00000fff, 0x00000c8a, 0x00003e00},
	{1, 0, 0, 0, 0, 0x00110842, 0x0000fbf1, 0x00000fff, 0x00000c8a, 0x00003e00}
};
#endif

static int brisket_cpu_order(int cpu)
{
	return cpu - BRISKET_CPU_START_ID;
}


int brisket_init_core(int cpu, unsigned int en)
{

#ifdef BRISKET_EN_CTRL_ATF
	int brisket_cpu;

	if ((cpu < BRISKET_CPU_START_ID) || (cpu > BRISKET_CPU_END_ID))
		return -1;

	brisket_cpu = brisket_cpu_order(cpu);

	/* coverity check */
	if ((brisket_cpu < 0) || (brisket_cpu >= NR_BRISKET_CPU)) {
		brisket_err("brisket_cpu(%d) is illegal\n", brisket_cpu);
		return -1;
	}

	/* update brisket_control */
	brisket_write((BRISKET_CPU_BASE[brisket_cpu] + BRISKET_CONTROL_OFFSET),
		brisket_info[brisket_cpu][BRISKET_GROUP_CONTROL]);
	brisket_debug("CPU[%d] BRISKET_GROUP_CONTROL=0x%08x\n",
		brisket_cpu, brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET_CONTROL_OFFSET));

	/* update brisket06 */
	brisket_write((BRISKET_CPU_BASE[brisket_cpu] + BRISKET06_OFFSET), brisket_info[brisket_cpu][BRISKET_GROUP_06]);
	brisket_debug("CPU[%d] BRISKET_GROUP_06=0x%08x\n",
		brisket_cpu, brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET06_OFFSET));

	/* update brisket07 */
	brisket_write((BRISKET_CPU_BASE[brisket_cpu] + BRISKET07_OFFSET), brisket_info[brisket_cpu][BRISKET_GROUP_07]);
	brisket_debug("CPU[%d] BRISKET_GROUP_07=0x%08x\n",
		brisket_cpu, brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET07_OFFSET));

	/* update brisket08 */
	brisket_write((BRISKET_CPU_BASE[brisket_cpu] + BRISKET08_OFFSET), brisket_info[brisket_cpu][BRISKET_GROUP_08]);
	brisket_debug("CPU[%d] BRISKET_GROUP_08=0x%08x\n",
		brisket_cpu, brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET08_OFFSET));

	/* update brisket09 */
	brisket_write((BRISKET_CPU_BASE[brisket_cpu] + BRISKET09_OFFSET), brisket_info[brisket_cpu][BRISKET_GROUP_09]);
	brisket_debug("CPU[%d] BRISKET_GROUP_09=0x%08x\n",
		brisket_cpu, brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET09_OFFSET));

	/* update brisket05 */
	if (en == 1) {
		brisket_write((BRISKET_CPU_BASE[brisket_cpu] + BRISKET05_OFFSET),
			brisket_info[brisket_cpu][BRISKET_GROUP_05]);
	} else {
		brisket_write((BRISKET_CPU_BASE[brisket_cpu] + BRISKET05_OFFSET),
			brisket_info[brisket_cpu][BRISKET_GROUP_05] & BRISKET05_MASK_Bren);
	}
	brisket_debug("CPU[%d] BRISKET_GROUP_05=0x%08x\n",
		brisket_cpu, brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET05_OFFSET));

#endif

	return 0;
}


int brisket_proc_read(unsigned int cpu, unsigned int brisket_group, unsigned int bits, unsigned int shift)
{
	int brisket_cpu;

	brisket_msg("cpu(%d), brisket_group(%d), bits(%d), shift(%d)\n", cpu, brisket_group, bits, shift);

	if ((cpu < BRISKET_CPU_START_ID) || (cpu > BRISKET_CPU_END_ID))
		return -1;

	brisket_cpu = brisket_cpu_order(cpu); /*adapt cpu number*/

	/* coverity check */
	if ((brisket_cpu < 0) || (brisket_cpu >= NR_BRISKET_CPU)) {
		brisket_err("brisket_cpu(%d) is illegal\n", brisket_cpu);
		return -1;
	}

	switch (brisket_group) {
	case BRISKET_GROUP_CONTROL:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET_CONTROL_OFFSET), bits, shift);
	case BRISKET_GROUP_01:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET01_OFFSET), bits, shift);
	case BRISKET_GROUP_02:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET02_OFFSET), bits, shift);
	case BRISKET_GROUP_03:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET03_OFFSET), bits, shift);
	case BRISKET_GROUP_04:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET04_OFFSET), bits, shift);
	case BRISKET_GROUP_05:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET05_OFFSET), bits, shift);
	case BRISKET_GROUP_06:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET06_OFFSET), bits, shift);
	case BRISKET_GROUP_07:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET07_OFFSET), bits, shift);
	case BRISKET_GROUP_08:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET08_OFFSET), bits, shift);
	case BRISKET_GROUP_09:
		return brisket_read_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET09_OFFSET), bits, shift);
	default:
		break;
	}

	return 0;
}

int brisket_proc_write(unsigned int cpu,
	unsigned int brisket_group, unsigned int bits, unsigned int shift, unsigned int value)
{
	int brisket_cpu;

	brisket_msg("cpu(%d), brisket_group(%d), bits(%d), shift(%d), value(%d)\n",
		cpu, brisket_group, bits, shift, value);

	if ((cpu < BRISKET_CPU_START_ID) || (cpu > BRISKET_CPU_END_ID))
		return -1;

	brisket_cpu = brisket_cpu_order(cpu); /*adapt cpu number*/

	/* coverity check */
	if ((brisket_cpu < 0) || (brisket_cpu >= NR_BRISKET_CPU)) {
		brisket_err("brisket_cpu(%d) is illegal\n", brisket_cpu);
		return -1;
	}

	switch (brisket_group) {
	case BRISKET_GROUP_CONTROL:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET_CONTROL_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_CONTROL] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET_CONTROL_OFFSET);
		break;
	case BRISKET_GROUP_01:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET01_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_01] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET01_OFFSET);
		break;
	case BRISKET_GROUP_02:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET02_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_02] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET02_OFFSET);
		break;
	case BRISKET_GROUP_03:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET03_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_03] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET03_OFFSET);
		break;
	case BRISKET_GROUP_04:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET04_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_04] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET04_OFFSET);
		break;
	case BRISKET_GROUP_05:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET05_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_05] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET05_OFFSET);
		break;
	case BRISKET_GROUP_06:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET06_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_06] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET06_OFFSET);
		break;
	case BRISKET_GROUP_07:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET07_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_07] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET07_OFFSET);
		break;
	case BRISKET_GROUP_08:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET08_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_08] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET08_OFFSET);
		break;
	case BRISKET_GROUP_09:
		brisket_write_bits((BRISKET_CPU_BASE[brisket_cpu] + BRISKET09_OFFSET), bits, shift, value);
		brisket_info[brisket_cpu][BRISKET_GROUP_09] =
			brisket_read(BRISKET_CPU_BASE[brisket_cpu] + BRISKET09_OFFSET);
		break;
	default:
		break;
	}

	return 0;
}

int brisket_control(unsigned int rw, unsigned int cpu, unsigned int brisket_group_bits_shift, unsigned int value)
{
	/*
	 * brisket_group_bits_shift = ( brisket_group << 16) | (bits << 8) | shift )
	 */
	unsigned int brisket_group = brisket_group_bits_shift >> 16;
	unsigned int bits = (brisket_group_bits_shift & 0xFF00) >> 8;
	unsigned int shift = brisket_group_bits_shift & 0xFF;

	brisket_msg("cpu(%d) brisket_group(%d) bits(%d) shift(%d) value(%d)\n",
		cpu, brisket_group, bits, shift, value);

	/* check for core if illegal */
	if ((cpu < BRISKET_CPU_START_ID) || (cpu > BRISKET_CPU_END_ID))
		return -1;

	/* check for bits,shift if illegal */
	if ((bits + shift) > 32)
		return -1;

	/* check for rw if illegal */
	if (rw >= NR_BRISKET_RW)
		return -1;

	/* check for brisket_group if illegal */
	if (brisket_group >= NR_BRISKET_GROUP)
		return -1;

	switch (rw) {
	case BRISKET_RW_READ:
		return brisket_proc_read(cpu, brisket_group, bits, shift);
	case BRISKET_RW_WRITE:
		brisket_proc_write(cpu, brisket_group, bits, shift, value);
		break;
	default:
		break;
	}

	return 0;
}

static unsigned int isFirstInitReady;

int brisket_init(unsigned int en)
{
	unsigned long mpidr;
	int cpu;

	/* for first init, config brisket in cpu0 */
	if (isFirstInitReady == 0) {
		brisket_init_core(0, 1);
		isFirstInitReady = 1;
	}

	/* hook brisket_init_core for specific core */
	mpidr = read_mpidr_el1();
	cpu = plat_core_pos_by_mpidr(mpidr);

	if ((cpu < BRISKET_CPU_START_ID) || (cpu > BRISKET_CPU_END_ID))
		return -1;

	brisket_init_core(cpu, en);

	return 0;
}

