/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DBG_CTRL_H__
#define __DBG_CTRL_H__

/* SPM_WAKEUP_MISC */
#define WAKE_MISC_TWAM		(1U << 18)
#define WAKE_MISC_PCM_TIMER	(1U << 19)
#define WAKE_MISC_CPU_WAKE	(1U << 20)

struct dbg_ctrl {
	uint32_t count;
	uint32_t duration;
	void *ext;
};

enum dbg_ctrl_enum {
	DBG_CTRL_COUNT,
	DBG_CTRL_DURATION,

	DBG_CTRL_MAX,
};

#endif /* __DBG_CTRL_H__ */
