/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EL3_UPTIME_H
#define EL3_UPTIME_H

#include <stdint.h>

/* Declaration for kernel time indicator */
extern unsigned int mt_log_ktime_sync;

#define MT_LOG_KTIME_CLEAR() { mt_log_ktime_sync = 0; }
#define MT_LOG_KTIME_SET() { mt_log_ktime_sync = 1; }
#define MT_LOG_KTIME mt_log_ktime_sync

void el3_uptime_init(uint64_t align_time_start, uint64_t el3_time_start);
uint64_t el3_uptime(void);
#endif /* EL3_UPTIME_H */
