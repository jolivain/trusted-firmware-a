/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTSPMC_H__
#define __MTSPMC_H__

int spmc_init(void);

void spm_poweron_cpu(int cluster, int cpu);
void spm_poweroff_cpu(int cluster, int cpu);

void spm_poweroff_cluster(int cluster);
void spm_poweron_cluster(int cluster);

int spm_get_cpu_powerstate(int cluster, int cpu);
int spm_get_cluster_powerstate(int cluster);
int spm_get_powerstate(uint32_t mask);

void mcucfg_init_archstate(int cluster, int cpu, int arm64);
void mcucfg_set_bootaddr(int cluster, int cpu, uintptr_t bootaddr);
uintptr_t mcucfg_get_bootaddr(int cluster, int cpu);

void mcucfg_disable_gic_wakeup(int cluster, int cpu);
void mcucfg_enable_gic_wakeup(int cluster, int cpu);

#endif /* __MTCMOS_H__ */
