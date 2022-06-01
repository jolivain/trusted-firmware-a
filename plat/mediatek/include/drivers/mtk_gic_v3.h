/*
 * Copyright (C) 2021 Mediatek Inc.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MTK_GIC_V3_H
#define MTK_GIC_V3_H

/* Extension event for platform driver */
#if VENDOR_EXTEND_PUBEVENT_ENABLE
#include <lib/el3_runtime/pubsub_events.h>

#define MT_SUBCRIBE_CHECK_WAKEUP_IRQ(_fn) \
	SUBSCRIBE_TO_EVENT(publish_check_wakeup_irq, _fn)
#define MT_PUBLISH_CHECK_WAKEUP_IRQ(x) ({\
	PUBLISH_EVENT_ARG(publish_check_wakeup_irq, (const void *)(x)); })
#else
#define MT_SUBCRIBE_CHECK_WAKEUP_IRQ(_fn)
#define MT_PUBLISH_CHECK_WAKEUP_IRQ(x) ({ (void)x; })
#endif

void gic_rdist_restore_all(void);
void gic_rdist_save(void);
void gic_rdist_restore(void);
void gic_dist_save(void);
void gic_dist_restore(void);
void gic_sgi_save_all(void);
void gic_sgi_restore_all(void);
unsigned int gicr_get_sgi_pending(void);

void gic_dpg_cfg_set(void);
void gic_dpg_cfg_clr(void);

void check_systimer_irq(int save);

void gic_dcm_restore(unsigned int val);
unsigned int gic_dcm_disabled_save(void);

#endif
