/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <console_macros.S>
#include <qti_uart_console.h>
#include <platform_def.h>

/*
 * This driver implements console logging into a ring buffer.
 */

	.globl qti_console_uart_register
	.globl console_uart_putc
	.globl console_uart_flush

	/* -----------------------------------------------
	 * int qti_console_uart_register(qti_console_uart_t *console,
	 *				 uintptr_t uart_base_addr)
	 * Registers uart console instance.
	 * In:  x0 - pointer to empty qti_console_uart_t struct
	 *      x1 - start address of uart block.
	 * Out: x0 - 1 to indicate success
	 * Clobber list: x0, x1, x14
	 * -----------------------------------------------
	 */
func qti_console_uart_register
	str	x1, [x0, #QTI_CONSOLE_T_UART_BASE_OFF]		/* Save UART base. */
	finish_console_register uart putc=1, flush=1
endfunc qti_console_uart_register

	/* -----------------------------------------------
	 * int qti_console_uart_puts(int c, qti_console_uart_t *console)
	 * Writes a character to the UART console.
	 * The character must be preserved in x0.
	 * In: x0 - character to be stored
	 *     x1 - pointer to qti_console_uart_t struct
	 * Clobber list: x1, x2
	 * -----------------------------------------------
	 */
func console_uart_putc
	/* set x1 = UART base. */
	ldr	x1, [x1, #QTI_CONSOLE_T_UART_BASE_OFF]

	/* Wait until transmit FIFO is empty. */
1:	ldr	w2, [x1, #MSM_BOOT_UART_DM_SR]
	and	w2, w2, #MSM_BOOT_UART_DM_SR_TXEMT
	cmp	w2, #MSM_BOOT_UART_DM_SR_TXEMT
	b.ne	1b

	/* Transmit data. */
	cmp	w0, #0xA
	b.ne	3f

	/* Add '\r' when input char is '\n' */
	mov	w2, #0x1
	/* add '\r' to w0 register */
	mov	w0, #0xD
	/* Transmit '\r' now */
	str	w2, [x1, #MSM_BOOT_UART_DM_NO_CHARS_FOR_TX]
	str	w0, [x1, #MSM_BOOT_UART_DM_TF]
	/* New line feed to w0 */
	mov	w0, #0xA

	/* Wait until transmit FIFO is empty. */
2:	ldr	w2, [x1, #MSM_BOOT_UART_DM_SR]
	and	w2, w2, #MSM_BOOT_UART_DM_SR_TXEMT
	cmp	w2, #MSM_BOOT_UART_DM_SR_TXEMT
	b.ne	2b

	/* Transmit i/p data. */
3:	mov	w2, #0x1
	str	w2, [x1, #MSM_BOOT_UART_DM_NO_CHARS_FOR_TX]
	str	w0, [x1, #MSM_BOOT_UART_DM_TF]

	ret
endfunc console_uart_putc

	/* -----------------------------------------------
	 * int qti_console_uart_flush(qti_console_uart_t *console)
	 * In:  x0 - pointer to qti_console_uart_t struct
	 * Out: x0 - 0 for success
	 * Clobber list: x0, x1
	 * now this is a stub function.
	 * -----------------------------------------------
	 */
func console_uart_flush
	/* set x0 = UART base. */
	ldr	x0, [x0, #QTI_CONSOLE_T_UART_BASE_OFF]

	/* Wait until transmit FIFO is empty. */
1:	ldr	w1, [x0, #MSM_BOOT_UART_DM_SR]
	and	w1, w1, #MSM_BOOT_UART_DM_SR_TXEMT
	cmp	w1, #MSM_BOOT_UART_DM_SR_TXEMT
	b.ne	1b

	mov	x0, #0
	ret
endfunc console_uart_flush
