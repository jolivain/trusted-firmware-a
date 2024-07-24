/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <drivers/delay_timer.h>

/*****************************************************************
 * Raspberry Pi 3 specific timer function to be used instead of
 * common platform delay timer functions.
 *****************************************************************/

#define SYSTEM_TIMER_BASE 0x3F003000
#define SYSTEM_TIMER_CLO (SYSTEM_TIMER_BASE + 0x04)

static inline uint32_t read_sys_timer() {
    return *((volatile uint32_t *)SYSTEM_TIMER_CLO);
}

uint64_t plat_timeout_init_us(uint32_t us)
{
    uint32_t start = read_sys_timer();
    uint32_t end = us;
    return (uint64_t)(start + end);
}

bool plat_timeout_elapsed(uint64_t expire_cnt)
{
    return read_sys_timer() > expire_cnt;
}

void plat_mdelay(uint32_t ms)
{
	uint32_t start = read_sys_timer();
	uint32_t delay_ticks = ms * 1000;
	while ((read_sys_timer() - start) < delay_ticks);
}

