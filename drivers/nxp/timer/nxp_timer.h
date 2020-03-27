/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* Ticks elapsed in one second by a signal of 1 KHz */
#define KHZ_TICKS_PER_SEC 1000

uint64_t get_timer_val(uint64_t start);

void delay_timer_init_args(uint32_t mult, uint32_t div);

/*
 * Initialise the nxp on-chip free rolling usec counter as the delay
 * timer.
 */
void delay_timer_init(void);
