/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <mt_cpu_pm.h>
#include <mt_lp_rm.h>
#include <mt_lp_irqremain.h>
#include <mt_lpm_dispatch.h>
#include <plat_mtk_lpm.h>
#include <plat_mt_cirq.h>
#include <plat_pm.h>
#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>

static struct mt_irqremain remain_irqs;
static struct mt_irqremain *pIrqs;

int mt_lp_irqremain_push(void)
{
	if (remain_irqs.count >= MT_IRQ_REMAIN_MAX) {
		return -1;
	}

	remain_irqs.count += 1;

	return 0;
}

int mt_lp_irqremain_pop(void)
{
	if (remain_irqs.count == 0) {
		return -1;
	}

	remain_irqs.count -= 1;

	return 0;
}

int mt_lp_irqremain_set(unsigned int type, const struct mt_lp_irqinfo *info)
{
	unsigned int idx;

	if (pIrqs || !info) {
		return -1;
	}

	idx = remain_irqs.count;

	switch (type) {
	case IRQS_REMAIN_IRQ:
		remain_irqs.irqs[idx] = info->val;
		break;
	case IRQS_REMAIN_WAKEUP_CAT:
		remain_irqs.wakeupsrc_cat[idx] = info->val;
		break;
	case IRQS_REMAIN_WAKEUP_SRC:
		remain_irqs.wakeupsrc[idx] = info->val;
		break;
	}

	return 0;
}

int mt_lp_irqremain_get(int idx, unsigned int type, struct mt_lp_irqinfo *info)
{
	if (!pIrqs || !info || idx < 0 || idx > remain_irqs.count) {
		return -1;
	}

	switch (type) {
	case IRQS_REMAIN_IRQ:
		info->val = remain_irqs.irqs[idx];
		break;
	case IRQS_REMAIN_WAKEUP_CAT:
		info->val = remain_irqs.wakeupsrc_cat[idx];
		break;
	case IRQS_REMAIN_WAKEUP_SRC:
		info->val = remain_irqs.wakeupsrc[idx];
		break;
	}

	return 0;
}

unsigned int mt_lp_irqremain_count(void)
{
	return remain_irqs.count;
}

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
