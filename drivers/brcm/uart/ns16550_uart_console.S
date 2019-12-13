/*
 * Copyright (c) 2015-2019, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#define USE_FINISH_CONSOLE_REG_2
#include <console_macros.S>
#include <drivers/brcm/uart/ns16550_uart.h>

	/*
	 * "core" functions are low-level implementations that don't require
	 * writable memory and are thus safe to call in BL1 crash context.
	 */
	.globl console_ns16550_core_init
	.globl console_ns16550_core_putc
	.globl console_ns16550_core_getc
	.globl console_ns16550_core_flush
	.globl console_ns16550_putc
	.globl console_ns16550_getc
	.globl console_ns16550_flush

	/* -----------------------------------------------
	 * int console_ns16550_core_init(uintptr_t base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x1, x2, x3
	 * -----------------------------------------------
	 */
func console_ns16550_core_init
	/* Check the input base address */
	cbz	x0, core_init_fail
	/* Check baud rate and uart clock for sanity */
	cbz	w1, core_init_fail
	cbz	w2, core_init_fail

	/* set UART_FCR_ISR , reset and disable RX/TX fifo */
	mov	w3, #(FCR_RX_FIFO_RESET | FCR_TX_FIFO_RESET)
	strb	w3, [x0, #UART_FCR_ISR]

	/* Wait for UART busy bit cleared */
	mov	w3, #UART_USR_BUSY_TIMEOUT
	mov	w4, #0
1:
	ldr	w5, [x0, #UART_USR]
	tbz	w5, #UART_USR_BUSY_BIT, 2f
	add	w4, w4, #0x1
	cmp	w4, w3
	b.le	1b
	b	core_init_timeout
2:

	/* set divisor latch access bit to set baud-rate */
	mov	w3, #UART_LCR_ENABLE_DIVISOR_LATCH;
	strb	w3, [x0, #UART_LCR]

	/* calculate divisor value */
	/*
	 * Divisor =  Uart clock / (16 * baudrate)
	 * Remainder =  Uart clock % (16 * baudrate)
	 *
	 * If Remainder > (16 * baudrate) / 2
	 *   Divisor += 1
	 */
	lsl	w2, w2, #4
	udiv	w3, w1, w2
	msub	w1, w3, w2, w1
	lsr	w2, w2, #1
	cmp	w1, w2
	cinc	w3, w3, hs
#if defined(BRCM_UART_DIVISOR_OVERRIDE)
	mov	w3, #1
#endif
	strb	w3, [x0, #UART_RXTXFIFO]

	/* set UART_IER */
	mov	w3, #0
	strb	w3, [x0, #UART_IER]

	/* setup line config and parity */
	mov	w3, #(LC_8_BIT_WORD_1 | PARITY_NONE)
	strb 	w3, [x0, #UART_LCR]

	/* Disable loop back */
	ldrb	w3, [x0, #UART_MCR]
	mov	w2, #0xef
	and	w3, w3, w2
	orr	w3, w3, #0x3
	strb	w3, [x0, #UART_MCR]

core_init_timeout:
	/* set UART_FCR_ISR, enable fifo */
	mov	w3, #(FCR_FIFO_ENABLE)
	strb	w3, [x0, #UART_FCR_ISR]

	mov	w0, #1
	ret
core_init_fail:
	mov	w0, wzr
	ret
endfunc console_ns16550_core_init

	.globl console_ns16550_register

	/* -----------------------------------------------
	 * int console_ns16550_register(uintptr_t baseaddr,
	 *                              uint32_t clock, uint32_t baud,
	 *                              console_ns16550_t *console);
	 * Function to initialize and register a new ns16550
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: x0 - UART register base address
	 *     w1 - UART clock in Hz
	 *     w2 - Baud rate
	 *     x3 - pointer to empty console_ns16550_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7, x14
	 * -----------------------------------------------
	 */
func console_ns16550_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_NS16550_BASE]

	bl	console_ns16550_core_init
	cbz	x0, register_fail

	mov	x0, x6
	mov	x30, x7
	finish_console_register ns16550 putc=1 getc=1 flush=1

register_fail:
	ret	x7
endfunc console_ns16550_register

	/* --------------------------------------------------------
	 * int console_ns16550_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x2, x3
	 * --------------------------------------------------------
	 */
func console_ns16550_core_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	cmp	w0, #0xA  /* check for new line char */
	b.ne	2f
1:
	/* wait fifo not full */
	ldrb	w2, [x1, #UART_USR]
	mov	w3, #UART_USR_TFNF
	and	w2, w2, w3
	cmp	w2, #0
	b.eq	1b

	/* send carriage return */
	mov	w2, #0xD
	strb	w2, [x1, #UART_RXTXFIFO]
2:
	/* wait fifo not full */
	ldrb	w2, [x1, #UART_USR]
	mov	w3, #UART_USR_TFNF
	and	w2, w2, w3
	cmp	w2, #0
	b.eq	2b

	/* send one char */
	strb	w0, [x1, #UART_RXTXFIFO]
	ret
endfunc console_ns16550_core_putc

	/* --------------------------------------------------------
	 * int console_ns16550_putc(int c, console_ns16550_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_ns16550_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x1, [x1, #CONSOLE_T_NS16550_BASE]
	b	console_ns16550_core_putc
endfunc console_ns16550_putc

	/* ---------------------------------------------
	 * int console_ns16550_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * In : x0 - console base address
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func console_ns16550_core_getc
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* check LSR for rx rdy bit */
1:	ldrb	w2, [x0, #UART_LSR]
	tbz	w2, #UART_LSR_RDY, 1b

	/* read the char */
	ldrb	w1, [x0, #UART_RXTXFIFO]
	mov	w0, w1
	ret
endfunc console_ns16550_core_getc

	/* ---------------------------------------------
	 * int console_ns16550_getc(console_ns16550_t *console)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In :  x0 - pointer to console_t structure
	 * Out : w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_ns16550_getc
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_NS16550_BASE]
	b	console_ns16550_core_getc
endfunc console_ns16550_getc

	/* ---------------------------------------------
	 * int console_ns16550_core_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_ns16550_core_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/*
	 * Loop until Transmit shift Register and
	 * Transmit FIFO both are Empty.
	 */
1:	ldr	w1, [x0, #UART_LSR]
	tbz	w1, #UART_LSR_TEMT, 1b

	mov	w0, #0
	ret
endfunc console_ns16550_core_flush

	/* ---------------------------------------------
	 * int console_ns16550_flush(console_ns16550_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - pointer to console_t structure
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_ns16550_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_NS16550_BASE]
	b	console_ns16550_core_flush
endfunc console_ns16550_flush
