/*
 * Copyright (c) 2015-2019, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DWC_UART_H
#define DWC_UART_H

/* definitions for UART */
#define FCR_FIFO_ENABLE		0x01
#define FCR_RX_FIFO_RESET	0x02
#define FCR_TX_FIFO_RESET	0x04

/* Baud Rate( based on clock 13Mhz ) */
#define BAUDRATE_115DOT2	0x07
#define BAUDRATE_9DOT6		0x01

/* Parity */
#define PARITY_ODD		0x08
#define PARITY_EVEN		0x18
#define PARITY_NONE		0x00

/* LineConfig */
#define LC_5_BIT_WORD_1		0x00
#define LC_6_BIT_WORD_1		0x01
#define LC_7_BIT_WORD_1		0x02
#define LC_8_BIT_WORD_1		0x03
#define LC_5_BIT_WORD_1P5	0x04
#define LC_6_BIT_WORD_2		0x05
#define LC_7_BIT_WORD_2		0x06
#define LC_8_BIT_WORD_2		0x07

#ifndef UART_REG_SHIFT
	#define UART_REG_SHIFT	2
#endif

/* registers map */
#define UART_RXTXFIFO	(0 << UART_REG_SHIFT) /* Rx/Tx Holding Register (DLL) */
#define UART_IER	(1 << UART_REG_SHIFT) /* Interrupt Enable Reg (DLM) */
#define UART_FCR_ISR	(2 << UART_REG_SHIFT) /* Fifo Cntrl/Intr Status Reg */
#define UART_LCR	(3 << UART_REG_SHIFT) /* Line Control Register */
#define UART_MCR	(4 << UART_REG_SHIFT) /* Modem Control Register */
#define UART_LSR	(5 << UART_REG_SHIFT) /* Line Status Register */
#define UART_MSR	(6 << UART_REG_SHIFT) /* Modem Status Register */
#define UART_SPR	(7 << UART_REG_SHIFT) /* Scratch Pad Register */
#define UART_USR	0x7C /* UART Status Register */

#define UART_USR_RFE	0x10 /* Recive FIFO empty */
#define UART_USR_RFNE	0x08 /* Recive FIFO not empty */
#define UART_USR_TFE	0x04 /* Transmit FIFO empty */
#define UART_USR_TFNF	0x02 /* Transmit FIFO not full */
#define UART_USR_BUSY	0x01 /* UART Busy (serial transfer in progress) */


#define UART_USR_BUSY_BIT	0 /* UART Busy bit */

/* UART Busy timeout value */
#define UART_USR_BUSY_TIMEOUT	10000

/* UART_LSR Bits */
#define UART_LSR_RDY		1 /* Data Ready */
/* Transmit holding register empty */
#define UART_LSR_THRE		5
/* Transmit Empty (Transmit shift register and TX FIFO, both empty) */
#define UART_LSR_TEMT		6

/* UART_LCR Bits */
#define UART_LCR_ENABLE_DIVISOR_LATCH	0x80

#define UART_INIT_OPTS		(FCR_RX_FIFO_RESET +	\
				 FCR_TX_FIFO_RESET +	\
				 FCR_FIFO_ENABLE)
#define UART_PARITY_OPTS	(LC_8_BIT_WORD_1 + PARITY_NONE)

#define CONSOLE_T_NS16550_BASE	CONSOLE_T_DRVDATA

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <drivers/console.h>

typedef struct {
	console_t console;
	uintptr_t base;
} console_ns16550_t;

/*
 * Initialize a new ns16550 console instance and register it with the console
 * framework. The |console| pointer must point to storage that will be valid
 * for the lifetime of the console, such as a global or static local variable.
 * Its contents will be reinitialized from scratch.
 */
int console_ns16550_register(uintptr_t baseaddr, uint32_t clock, uint32_t baud,
			     console_ns16550_t *console);
void console_ns16550_core_flush(uint32_t uart_baseaddr);

#endif /*__ASSEMBLER__*/
#endif /* DWC_UART_H */
