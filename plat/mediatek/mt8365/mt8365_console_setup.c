/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/console.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 || (__x) > 0) ?	\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)

struct mtk_uart_regs {
	union {
		uint32_t rbr;
		uint32_t thr;
	};
	uint32_t ier;
	uint32_t fcr;
	uint32_t lcr;
	uint32_t mcr;
	uint32_t lsr;
	uint32_t reserved0[3];
	uint32_t highspeed;
	uint32_t sample_count;
	uint32_t sample_point;
	uint32_t reserved1[9];
	uint32_t fracdiv_l;
	uint32_t fracdiv_m;
	uint32_t reserved2[13];
	uint32_t dll;
	uint32_t dlm;
};

#define UART_LCR_WLS_8	 0x03	 /* 8 bit character length */
#define UART_LCR_DLAB	 BIT(7)	 /* Divisor latch access bit */

#define UART_LSR_DR	 BIT(0)	 /* Data ready */
#define UART_LSR_THRE	 BIT(5)	 /* Xmit holding register empty */
#define UART_LSR_TEMT	 BIT(6)	 /* Xmitter empty */

#define UART_FCR_FIFO_EN BIT(0)	 /* Fifo enable */
#define UART_FCR_RXSR	 BIT(1)	 /* Receiver soft reset */
#define UART_FCR_TXSR	 BIT(2)	 /* Transmitter soft reset */

/* Clear & enable FIFOs */
#define UART_FCRVAL (UART_FCR_FIFO_EN | \
		     UART_FCR_RXSR |	\
		     UART_FCR_TXSR)

static int mtk_console_putc(int character, struct console *console);
static void mtk_console_flush(struct console *console);

static console_t mtk_console = {
	.flags = CONSOLE_FLAG_BOOT |
		 CONSOLE_FLAG_RUNTIME |
		 CONSOLE_FLAG_CRASH |
		 CONSOLE_FLAG_TRANSLATE_CRLF,
	.putc = mtk_console_putc,
	.flush = mtk_console_flush,
	.base = MTK_UART_DEFAULT_BASE,
};

#define mtk_uart_write(reg, value) \
	mmio_write_32((uintptr_t)&((struct mtk_uart_regs *)mtk_console.base)->reg, value)

#define mtk_uart_read(reg) \
	mmio_read_32((uintptr_t)&((struct mtk_uart_regs *)mtk_console.base)->reg)

static void mtk_uart_setbrg(unsigned int baud)
{
	const unsigned int uartclk = MTK_UART_CLOCK;

	unsigned int byte;
	unsigned int highspeed;
	unsigned int divisor;
	unsigned int sample_count, sample_point, fraction;

	unsigned short fraction_L_mapping[] = {
		0, 1, 0x5, 0x15, 0x55, 0x57, 0x57, 0x77, 0x7F, 0xFF, 0xFF
	};

	unsigned short fraction_M_mapping[] = {
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 3
	};

	if (baud <= 115200) {
		highspeed = 0;
		divisor = DIV_ROUND_CLOSEST(uartclk, 16 * baud);
	} else {
		highspeed = 3;
		divisor = div_round_up(uartclk, 256 * baud);

		sample_count = (uartclk / (divisor * baud)) - 1;
		sample_point = (sample_count >> 1) - 1;

		mtk_uart_write(sample_count, sample_count);
		mtk_uart_write(sample_point, sample_point);

		fraction = ((uartclk  * 100) / baud / divisor) % 100;
		fraction = DIV_ROUND_CLOSEST(fraction, 10);

		mtk_uart_write(fracdiv_l, fraction_L_mapping[fraction]);
		mtk_uart_write(fracdiv_m, fraction_M_mapping[fraction]);
	}

	mtk_uart_write(highspeed, highspeed);

	byte = mtk_uart_read(lcr);
	mtk_uart_write(lcr, (byte | UART_LCR_DLAB));
	mtk_uart_write(dll, (divisor & 0xff));
	mtk_uart_write(dlm, ((divisor >> 8) & 0xff));
	mtk_uart_write(lcr, byte);
}

static int mtk_console_putc(int character, struct console *console)
{
	while (!(mtk_uart_read(lsr) & UART_LSR_THRE))
		;

	mtk_uart_write(thr, (uint32_t)character);

	return 0;
}

static void mtk_console_flush(struct console *console)
{
	while (!(mtk_uart_read(lsr) & UART_LSR_TEMT))
		;
}

void mtk_console_register(void)
{
	mtk_uart_write(fcr, UART_FCRVAL);
	mtk_uart_write(lcr, UART_LCR_WLS_8);

	mtk_uart_setbrg(MTK_UART_BAUDRATE);

	console_register(&mtk_console);
}
