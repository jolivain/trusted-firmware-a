/*
 * Copyright (c) 2018, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WDT_H
#define WDT_H

void wdt_pet(void);
void wdt_resume(void);
void wdt_set_enable(int enable);
int wdt_set_timeout(uint32_t timeout);
uint64_t wdt_smc_handler(uint32_t smc_fid, uint32_t x1, uint32_t x2,
	uint32_t x3, uint32_t x4, void *cookie, void *handle, uint64_t flags);
void wdt_suspend(void);
void wdt_trigger_reset(void);

#endif /* WDT_H */
