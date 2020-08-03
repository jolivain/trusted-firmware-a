/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_SPM_H__
#define __MT_SPM_H__

#include <stdio.h>
#include <stdint.h>
#include <lib/bakery_lock.h>
#include <lib/spinlock.h>
#include <plat_mtk_lpm.h>

/* ARM v8.2, the cache will turn off automatically when cpu
 * power down. So, there is no doubt to use the spin_lock here
 */
#if !HW_ASSISTED_COHERENCY
#define MT_SPM_USING_BAKERY_LOCK
#endif

#ifdef MT_SPM_USING_BAKERY_LOCK
DECLARE_BAKERY_LOCK(spm_lock);
#define plat_spm_lock()\
	bakery_lock_get(&spm_lock)

#define plat_spm_unlock()\
	bakery_lock_release(&spm_lock)
#else
extern spinlock_t spm_lock;
#define plat_spm_lock()\
	spin_lock(&spm_lock)

#define plat_spm_unlock()\
	spin_unlock(&spm_lock)
#endif

#ifdef __GNUC__
#define spm_likely(x)   __builtin_expect(!!(x), 1)
#define spm_unlikely(x) __builtin_expect(!!(x), 0)
#else
#define spm_likely(x)   (x)
#define spm_unlikely(x) (x)
#endif

#define SPM_FW_NO_RESUME 1
#define MCUSYS_MTCMOS_ON 0
#define WAKEUP_LOG_ON	 0

#define MT_SPM_USING_SRCLKEN_RC
/* spm extern operand definition */
#define MT_SPM_EX_OP_CLR_26M_RECORD                             (1 << 0UL)
#define MT_SPM_EX_OP_SET_WDT                                    (1 << 1UL)
#define MT_SPM_EX_OP_NON_GENERIC_RESOURCE_REQ                   (1 << 2UL)
#define MT_SPM_EX_OP_SET_SUSPEND_MODE                           (1 << 3UL)
#define MT_SPM_EX_OP_SET_IS_ADSP				(1 << 4UL)
#define MT_SPM_EX_OP_SRCLKEN_RC_BBLPM				(1 << 5UL)
#define MT_SPM_EX_OP_HW_S1_DETECT				(1 << 6UL)
#define MT_SPM_EX_OP_TRACE_LP					(1 << 7UL)
#define MT_SPM_EX_OP_TRACE_SUSPEND				(1 << 8UL)

/* EN SPM INFRA DEBUG OUT */
#define DEBUGSYS_DEBUG_EN_REG (DBGSYS_DEM_BASE + 0x94)

/* INFRA_AO_DEBUG_CON */
#define INFRA_AO_DBG_CON0 (INFRACFG_AO_BASE + 0x500)
#define INFRA_AO_DBG_CON1 (INFRACFG_AO_BASE + 0x504)
#define INFRA_AO_DBG_CON2 (INFRACFG_AO_BASE + 0x508)
#define INFRA_AO_DBG_CON3 (INFRACFG_AO_BASE + 0x50C)

/* AP_MDSRC_REQ MD 26M ON settle time (3ms) */
#define AP_MDSRC_REQ_MD_26M_SETTLE (3)

/* Setting the SPM settle time*/
#define SPM_SYSCLK_SETTLE       0x60FE	/* 1685us */

typedef enum {
	WR_NONE = 0,
	WR_UART_BUSY = 1,
	WR_ABORT = 2,
	WR_PCM_TIMER = 3,
	WR_WAKE_SRC = 4,
	WR_DVFSRC = 5,
	WR_TWAM = 6,
	WR_PMSR = 7,
	WR_SPM_ACK_CHK = 8,
	WR_UNKNOWN = 9,
} wake_reason_t;

struct pwr_ctrl;

void spm_boot_init(void);
void spm_set_bootaddr(unsigned long bootaddr);

static inline void spm_lock_get(void)
{
	plat_spm_lock();
}

static inline void spm_lock_release(void)
{
	plat_spm_unlock();
}

#endif /* __MT_SPM_H__ */
