/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <asm_macros.S>
#include <assert_macros.S>
#include <console_macros.S>
#include <common/drivers/uart/uart8250.h>

	.globl	console_8250_putc
	.globl	console_8250_getc
	.globl	console_8250_flush
	.globl	console_8250_register

	/* -----------------------------------------------
	 * int console_core_init(unsigned long base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : x1, x2, x3
	 *
	 * C code snippet:
	 * mmio_write_16((UART_LCR(uart_base)), UART_WLS_8);
	 * data = (unsigned short)(uart_clock/baudrate);
	 * ;;divided by 256
	 * high_speed_div = (data >> 8) + 1;
	 * tmp_div = uart_clock / (baudrate*high_speed_div);
	 * divisor =  (unsigned short)tmp_div;
	 * remainder = (uart_clock) % (high_speed_div*baudrate);
	 * ;;get (sample_count+1)
	 * if (remainder >= ((baudrate)*(high_speed_div))>>1)
	 * 	divisor =  (unsigned short)(tmp_div+1);
	 * else
	 * 	divisor =  (unsigned short)tmp_div;
	 * sample_count = divisor-1;
	 * ;;get the sample point
	 * sample_point = (sample_count - 1) >> 1;
	 * ;;configure register
	 * mmio_write_32(UART_HIGHSPEED(uart_base), 0x3);
	 * byte = mmio_read_32(UART_LCR(uart_base));
	 * ;;DLAB start
	 * mmio_write_32(UART_LCR(uart_base), (byte | UART_LCR_DLAB));
	 * mmio_write_32(UART_DLL(uart_base), (high_speed_div & 0x00ff));
	 * mmio_write_32(UART_DLH(uart_base), ((high_speed_div >> 8) & 0x00ff));
	 * mmio_write_32(UART_SAMPLE_COUNT(uart_base), sample_count);
	 * mmio_write_32(UART_SAMPLE_POINT(uart_base), sample_point);
	 * ;;DLAB End
	 * mmio_write_32(UART_LCR(uart_base), byte);
	 * -----------------------------------------------
	 */
func console_core_init
	/* Check the input base address */
	cbz	x0, core_init_fail
	/* Check baud rate and uart clock for sanity */
	cbz	w1, core_init_fail
	cbz	w2, core_init_fail

	/* Disable interrupt */
	str	wzr, [x0, #UART_IER]
	/* Clear fractional divider */
	str	wzr, [x0, #UART_FRACDIV_L]
	str	wzr, [x0, #UART_FRACDIV_M]
	/* Force DTR and RTS to high */
	mov	w3, #(UART_MCR_DTR | UART_MCR_RTS)
	str	w3, [x0, #UART_MCR]
	/* Check high speed */
	movz	w3, #:abs_g1:115200
	movk	w3, #:abs_g0_nc:115200
	cmp	w2, w3
	b.hi	1f
	/* Non high speed */
	lsl	w2, w2, #4
	mov	w3, wzr
	b	2f
	/* High speed */
1:	mov	w3, #3
	/* Set high speed UART register */
2:	str	w3, [x0, #UART_HIGHSPEED]

	/* Setup N81,(UART_WLS_8 | UART_NONE_PARITY | UART_1_STOP) = 0x03 */
	mov	w8, #UART_LCR_WLS_8
	strh	w8, [x0, #UART_LCR]
	cmp w3, #3
	beq	3f
	/* Calculate divisor */
	udiv	w3, w1, w2	/* divisor = uartclk / (quot * baudrate) */
	msub	w1, w3, w2, w1	/* remainder = uartclk % (quot * baudrate) */
	lsr	w2, w2, #1
	cmp	w1, w2
	cinc	w3, w3, hs

	/* Set line configuration, access divisor latches */
	mov	w8, #(UART_LCR_DLAB | UART_LCR_WLS_8)
	strh	w8, [x0, #UART_LCR]

	/* Set the divisor */
	and	w1, w3, #0xff
	str	w1, [x0, #UART_DLL]
	lsr	w1, w3, #8
	and	w1, w1, #0xff
	str	w1, [x0, #UART_DLH]

	/* Hide the divisor latches */
	mov	w1, #UART_LCR_WLS_8
	str	w1, [x0, #UART_LCR]

	/* Enable FIFOs, and clear receive and transmit */
	mov	w1, #(UART_FCR_FIFO_EN | UART_FCR_CLEAR_RCVR |	\
			UART_FCR_CLEAR_XMIT)
	str	w1, [x0, #UART_FCR]
	b	core_init_ok
3:
	udiv	w9, w1, w2
	mov	w4, #256
	udiv    w9, w9, w4
	/* w8 = high_speed_div */
	add	w8, w9, #1
	/* w10 = baudrate*high_speed_div */
	mul	w10, w8, w2
	/* w11 = divisor */
	udiv	w11, w1, w10
	/*w12 = remainder */
	msub	w12, w11, w10, w1
	cmp	w12, w10, lsr #1
	/* w11 = divisor */
	cinc	w11, w11, cs
	sub	w11, w11, #0x1
	str	w3, [x0, #UART_HIGHSPEED]
	/* DLAB start */
	/* w9 = byte */
	ldr	w9, [x0,#UART_LCR]
	and	w9, w9, #0xffff
	orr	w4, w9, #UART_LCR_DLAB
	str	w4, [x0,#UART_LCR]
	and	w10, w8, #0xff
	str	w10, [x0]
	lsr	w8, w8, #8
	and	w8, w8, #0xff
	str	w8, [x0,#UART_DLH]
	and	w8, w11, #0xffff
	str	w8, [x0,#UART_SAMPLE_COUNT]
	sub	w10, w8, #0x1
	ubfx	w8, w10, #1, #16
	str	w8, [x0,#UART_SAMPLE_POINT]
	/* DLAB End */
	str	w9, [x0,#UART_LCR]
core_init_ok:
	mov w0, #1
	ret
core_init_fail:
	mov	w0, wzr
	ret
endfunc console_core_init

func console_8250_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x1, [x1, #CONSOLE_T_BASE]
	b console_8250_core_putc
endfunc console_8250_putc

	/* --------------------------------------------------------
	 * int console_8250_core_putc(int c, unsigned long base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_8250_core_putc
	/* Check the input parameter */
	cbz	x1, putc_error
	/* Prepend '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f

	/* Check if the transmit FIFO is full */
1:	ldr	w2, [x1, #UART_LSR]
	and	w2, w2, #UART_LSR_THRE
	cbz	w2, 1b
	mov	w2, #0xD
	str	w2, [x1, #UART_THR]

	/* Check if the transmit FIFO is full */
2:	ldr	w2, [x1, #UART_LSR]
	and	w2, w2, #UART_LSR_THRE
	cbz	w2, 2b
	str	w0, [x1, #UART_THR]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_8250_core_putc

func console_8250_getc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x1, [x1, #CONSOLE_T_BASE]
	b console_8250_core_getc
endfunc console_8250_getc

	/* ---------------------------------------------
	 * int console_8250_core_getc(unsigned long base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * In : x0 - console base address
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_8250_core_getc
	cbz	x0, getc_error

	/* Check if the receive FIFO is empty */
1:	ldr	w1, [x0, #UART_LSR]
	tbz	w1, #UART_LSR_DR, 1b
	ldr	w0, [x0, #UART_RBR]
	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_8250_core_getc

func console_8250_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_BASE]
	b console_8250_core_flush
endfunc console_8250_flush

	/* ---------------------------------------------
	 * void console_8250_core_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Out : void.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_8250_core_flush
	/* Check if the transmit FIFO is empty */
1:	ldr	w1, [x0, #UART_LSR]
	and	w1, w1, #(UART_LSR_THRE | UART_LSR_TEMT)
	cbz	w1, 1b
	ret
endfunc console_8250_core_flush

	/* -----------------------------------------------
	 * int console_8250_register(uintptr_t baseaddr,
	 *     uint32_t clock, uint32_t baud,
	 *     console_t *console);
	 * Function to initialize and register a new 8250
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * If w1 (UART clock) is 0, initialisation will be
	 * skipped, relying on previous code to have done
	 * this already. w2 is ignored then as well.
	 * In: x0 - UART register base address
	 *     w1 - UART clock in Hz
	 *     w2 - Baud rate (ignored if w1 is 0)
	 *     x3 - pointer to empty console_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7, x14
	 * -----------------------------------------------
	 */
func console_8250_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_BASE]

	/* A clock rate of zero means to skip the initialisation. */
	cbz	w1, register_8250

	bl	console_core_init
	cbz	x0, register_fail

register_8250:
	mov	x0, x6
	mov	x30, x7
	finish_console_register 8250 putc=1, flush=1

register_fail:
	ret	x7
endfunc console_8250_register
