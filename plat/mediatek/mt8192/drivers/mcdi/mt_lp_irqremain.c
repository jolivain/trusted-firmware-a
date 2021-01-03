/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mt_lp_rm.h>
#include <mt_lp_irqremain.h>
#include <plat_mtk_lpm.h>
#include <plat_mt_cirq.h>

static struct mt_irqremain remain_irqs;
static struct mt_irqremain *pIrqs;

int mt_lp_irqremain_submit(void)
{
	if (remain_irqs.count == 0) {
		return -1;
	}

	set_wakeup_sources(remain_irqs.irqs, remain_irqs.count);
	mt_lp_rm_do_update(-1, PLAT_RC_UPDATE_REMAIN_IRQS, &remain_irqs);
	pIrqs = &remain_irqs;

	return 0;
}

int mt_lp_irqremain_aquire(void)
{
	if (!pIrqs) {
		return -1;
	}

	mt_cirq_sw_reset();
	mt_cirq_clone_gic();
	mt_cirq_enable();

	return 0;
}

int mt_lp_irqremain_release(void)
{
	if (!pIrqs) {
		return -1;
	}

	mt_cirq_flush();
	mt_cirq_disable();

	return 0;
}

void mt_lp_irqremain_init(void)
{
	uint32_t idx;

	pIrqs = NULL;
	remain_irqs.count = 0;

	/* level edma0 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 448;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mdla */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 446;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali4 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 399;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali3 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 398;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali2 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 397;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali1 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 396;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali0 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 395;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level vpu core1 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 453;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level vpu core0 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 452;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* edge mdwdt */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 110;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0x2000000;
	remain_irqs.count++;

	/* edge keypad */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = 106;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0x4;
	remain_irqs.count++;

	mt_lp_irqremain_submit();
}
