/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <lib/mmio.h>

#include <mt_spm_notifier.h>
#include <mt_spm_sspm_intc.h>

#define MT_SPM_SSPM_MBOX_OFF(x)		(SSPM_MBOX_BASE + x)
#define MT_SPM_MBOX(slot)		MT_SPM_SSPM_MBOX_OFF((slot << 2UL))

#define SSPM_MBOX_SPM_LP_LOOKUP1	MT_SPM_MBOX(0)
#define SSPM_MBOX_SPM_LP_LOOKUP2	MT_SPM_MBOX(1)
#define SSPM_MBOX_SPM_LP1		MT_SPM_MBOX(2)
#define SSPM_MBOX_SPM_LP2		MT_SPM_MBOX(3)

#define DO_SPM_SSPM_LP_SUSPEND()	\
	mmio_write_32(SPM_MD32_IRQ, MT_SPM_SSPM_INTC0_HIGH)
#define DO_SPM_SSPM_LP_RESUME()		\
	mmio_write_32(SPM_MD32_IRQ, MT_SPM_SSPM_INTC0_LOW)

int mt_spm_sspm_notify(int type, unsigned int lp_mode)
{
	switch (type) {
	case MT_SPM_NOTIFY_LP_ENTER:
		mmio_write_32(SSPM_MBOX_SPM_LP1, lp_mode);
		DO_SPM_SSPM_LP_SUSPEND();
		break;
	case MT_SPM_NOTIFY_LP_LEAVE:
		mmio_write_32(SSPM_MBOX_SPM_LP1, lp_mode);
		DO_SPM_SSPM_LP_RESUME();
		break;
	default:
		break;
	}

	return 0;
}
