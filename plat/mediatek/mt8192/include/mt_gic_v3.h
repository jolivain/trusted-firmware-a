/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_GIC_V3_H
#define MT_GIC_V3_H

#include <lib/mmio.h>

void mt_gic_driver_init(void);
void mt_gic_init(void);
void mt_gic_set_pending(uint32_t irq);
uint32_t mt_gic_get_pending(uint32_t irq);
void mt_gic_cpuif_enable(void);
void mt_gic_cpuif_disable(void);
void mt_gic_rdistif_init(void);
void mt_gic_distif_save(void);
void mt_gic_distif_restore(void);
void mt_gic_rdistif_save(void);
void mt_gic_rdistif_restore(void);

#endif /* MT_GIC_V3_H */
