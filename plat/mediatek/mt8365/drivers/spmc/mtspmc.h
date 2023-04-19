/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTSPMC_H
#define MTSPMC_H

int spmc_init(void);

void spm_poweroff_cluster(uint32_t cluster);
void spm_poweron_cluster(uint32_t cluster);

bool spm_get_cluster_powerstate(uint32_t cluster);
bool spm_get_powerstate(uint32_t mask);

void mcucfg_init_archstate(uint32_t cluster, uint32_t cpu, bool arm64);
void mcucfg_set_bootaddr(uint32_t cluster, uint32_t cpu, uintptr_t bootaddr);
uintptr_t mcucfg_get_bootaddr(uint32_t cluster, uint32_t cpu);

#endif /* MTSPMC_H */
