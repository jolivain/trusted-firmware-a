/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_LP_IRQREMAIN_H__
#define __MT_LP_IRQREMAIN_H__

struct mt_lp_irqinfo {
	unsigned int val;
};

enum MT_LP_IRQREMAIN_TYPE {
	MT_LP_IRQREMAIN_IRQ,
	MT_LP_IRQREMAIN_WAKEUP_CAT,
	MT_LP_IRQREMAIN_WAKEUP_SRC,
};

int mt_lp_irqremain_set(unsigned int type, const struct mt_lp_irqinfo *value);
int mt_lp_irqremain_get(int idx, unsigned int type,
			struct mt_lp_irqinfo *value);
unsigned int mt_lp_irqremain_count(void);
int mt_lp_irqremain_push(void);
int mt_lp_irqremain_pop(void);
int mt_lp_irqremain_submit(void);
int mt_lp_irqremain_aquire(void);
int mt_lp_irqremain_release(void);
void mt_lp_irqremain_init(void);
#endif /* __MT_LP_IRQREMAIN_H__ */
