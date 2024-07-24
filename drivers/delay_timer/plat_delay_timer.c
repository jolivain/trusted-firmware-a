/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <drivers/delay_timer.h>

/*************************************************************
 * This plat delay timer driver allows for platforms that lack 
 * ARM_SYS_TIMCTL_BASE to use the the same function call to as 
 * platforms that use the delay timer to allow for wider use.
 *************************************************************/

uint64_t plat_timeout_init_us(uint32_t us)
{
    return timeout_init_us(us);
}

bool plat_timeout_elapsed(uint64_t expire_cnt)
{
    return timeout_elapsed(expire_cnt);
}

void plat_mdelay(uint32_t ms)
{
    mdelay(ms);
}

