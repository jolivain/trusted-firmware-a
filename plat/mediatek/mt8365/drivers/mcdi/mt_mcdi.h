/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_MCDI_H
#define MT_MCDI_H

void mcupm_init(void);
void mcupm_hotplug_on(int cpu);
void mcupm_hotplug_off(int cpu);

#endif /* MT_MCDI_H */
