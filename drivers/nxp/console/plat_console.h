/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_CONSOLE_H
#define PLAT_CONSOLE_H

/*
 * Function to initialize platform's console
 * and register with console framework
 */
void plat_console_init(uintptr_t nxp_console_addr, uint32_t uart_clk_div,
			uint32_t baud);

#endif
