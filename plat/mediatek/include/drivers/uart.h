/*
 * Copyright (c) 2021, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

int plat_uart_init(uintptr_t uart_id, uint32_t baudrate, console_t *console);
void mtk_uart_restore(void);
void mtk_uart_save(void);
uint32_t mtk_uart_wakeup(void);
uint32_t mtk_uart_sleep(void);

#endif
