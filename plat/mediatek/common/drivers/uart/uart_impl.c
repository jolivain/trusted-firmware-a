/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <common/drivers/uart/uart8250.h>
#include <common/drivers/uart/uart_def.h>
#include <drivers/uart.h>
#include <mtk_mmap_pool.h>
#include <platform_def.h>

#define UNUSED_PARAM(param)	((void)param)

struct mtk_uart uart_save_addr[HW_SUPPORT_UART_PORTS];

static const mmap_region_t uart_mmap[] MTK_MMAP_SECTION = {
#ifdef UART0_BASE
	MAP_REGION_FLAT(UART0_BASE, PAGE_SIZE,
	MT_DEVICE | MT_RW | MT_SECURE),
#endif
#ifdef UART1_BASE
	MAP_REGION_FLAT(UART1_BASE, PAGE_SIZE,
	MT_DEVICE | MT_RW | MT_SECURE),
#endif
	{0}
};
DECLARE_MTK_MMAP_REGIONS(uart_mmap);

int plat_uart_init(uintptr_t uart_base, uint32_t baudrate, console_t *console)
{
	assert(uart_base != 0);

	return console_8250_register(uart_base,
				 UART_CLOCK,
				 baudrate,
				 console);
}

void mtk_uart_restore(void)
{
	uint32_t base;
	int uart_idx = 0;
	struct mtk_uart *uart;

	/* Attention!! Must NOT print any debug log before UART restore */
	for (uart_idx = 0; uart_idx < HW_SUPPORT_UART_PORTS; uart_idx++) {
		uart = &uart_save_addr[uart_idx];
		assert(uart != NULL);
		base = uart->base;
		assert(base != 0);

#if defined(PERI_CG0_STA) && defined(UART0_CG_BIT)
		if ((mmio_read_32(PERI_CG0_STA) & (UART0_CG_BIT << uart_idx)))
			continue;
#endif

		mmio_write_32(UART_FCR_DRV(base), uart->registers.fcr);
		mmio_write_32(UART_LCR_DRV(base), uart->registers.lcr);
		/* baudrate */
		mmio_write_32(UART_HIGHSPEED_DRV(base), uart->registers.highspeed);
		mmio_write_32(UART_FRACDIV_L_DRV(base), uart->registers.fracdiv_l);
		mmio_write_32(UART_FRACDIV_M_DRV(base), uart->registers.fracdiv_m);

		mmio_write_32(UART_FEATURE_SEL(base), UART_NEW_MAP_ENABLE);
		mmio_write_32(UART_EFR(base), uart->registers.efr);
		mmio_write_32(UART_DLL_NEW(base), uart->registers.dll);
		mmio_write_32(UART_DLH_NEW(base), uart->registers.dlh);
		mmio_write_32(UART_FEATURE_SEL(base), ~UART_NEW_MAP_ENABLE);

		mmio_write_32(UART_SAMPLE_COUNT_DRV(base), uart->registers.sample_count);
		mmio_write_32(UART_SAMPLE_POINT_DRV(base), uart->registers.sample_point);
		mmio_write_32(UART_GUARD_DRV(base), uart->registers.guard);

		/* flow control */
		mmio_write_32(UART_ESCAPE_EN_DRV(base), uart->registers.escape_en);
		mmio_write_32(UART_MCR_DRV(base), uart->registers.mcr);
		mmio_write_32(UART_SCR_DRV(base), uart->registers.scr);
		mmio_write_32(UART_IER_DRV(base), uart->registers.ier);
	}
}

void mtk_uart_save(void)
{
	int uart_idx = 0;
	uint32_t base;
	struct mtk_uart *uart;

	for (uart_idx = 0; uart_idx < HW_SUPPORT_UART_PORTS; uart_idx++) {
		uart_save_addr[uart_idx].base = uart_mmap[uart_idx].base_pa;
		base = uart_save_addr[uart_idx].base;
		assert(base != 0);
		uart = &uart_save_addr[uart_idx];
		assert(uart != NULL);

#if defined(PERI_CG0_STA) && defined(UART0_CG_BIT)
		if ((mmio_read_32(PERI_CG0_STA) & (UART0_CG_BIT << uart_idx)))
			continue;
#endif

		uart->registers.lcr = mmio_read_32(UART_LCR_DRV(base));

		/* baudrate */
		uart->registers.highspeed = mmio_read_32(UART_HIGHSPEED_DRV(base));
		uart->registers.fracdiv_l = mmio_read_32(UART_FRACDIV_L_DRV(base));
		uart->registers.fracdiv_m = mmio_read_32(UART_FRACDIV_M_DRV(base));

		mmio_write_32(UART_FEATURE_SEL(base), UART_NEW_MAP_ENABLE);
		/*to avoid LCR[7]==1 cause DLL is overwritten*/
		uart->registers.efr = mmio_read_32(UART_EFR(base));
		uart->registers.fcr = mmio_read_32(UART_FCR_RD_DRV(base));

		uart->registers.dll = mmio_read_32(UART_DLL_NEW(base));
		uart->registers.dlh = mmio_read_32(UART_DLH_NEW(base));
		mmio_write_32(UART_FEATURE_SEL(base), ~UART_NEW_MAP_ENABLE);

		uart->registers.sample_count = mmio_read_32(UART_SAMPLE_COUNT_DRV(base));
		uart->registers.sample_point = mmio_read_32(UART_SAMPLE_POINT_DRV(base));
		uart->registers.guard = mmio_read_32(UART_GUARD_DRV(base));

		/* flow control */
		uart->registers.escape_en = mmio_read_32(UART_ESCAPE_EN_DRV(base));
		uart->registers.mcr = mmio_read_32(UART_MCR_DRV(base));
		uart->registers.scr = mmio_read_32(UART_SCR_DRV(base));
		uart->registers.ier = mmio_read_32(UART_IER_DRV(base));
	}
}

uint32_t mtk_uart_sleep(void)
{
	uint32_t uart_idx = 0;
	uint32_t base;
	uint32_t sleep_req = 0, i = 0;

	for (uart_idx = 0; uart_idx < HW_SUPPORT_UART_PORTS; uart_idx++) {

		base = uart_mmap[uart_idx].base_pa;
		assert(base != 0);
		/* request UART to sleep */
		sleep_req = mmio_read_32(UART_SLEEP_REQ_DRV(base));
		mmio_write_32(UART_SLEEP_REQ_DRV(base),
		    sleep_req | MTK_UART_SEND_SLEEP_REQ);

		/* wait for UART to ACK */
		while (!(mmio_read_32(UART_SLEEP_ACK_DRV(base))
			& MTK_UART_SLEEP_ACK_IDLE)) {
			if (i++ >= MTK_UART_WAIT_ACK_TIMES) {
				mmio_write_32(UART_SLEEP_REQ_DRV(base), sleep_req);
				ERROR("CANNOT GET UART%d SLEEP ACK\n", uart_idx);

				return UART_PM_ERROR;
			}

			udelay(10);
		}
	}
	return 0;
}

uint32_t mtk_uart_wakeup(void)
{
	uint32_t i = 0;
	uint32_t sleep_req = 0;
	uint32_t uart_idx = 0;
	uint32_t base;

	for (uart_idx = 0; uart_idx < HW_SUPPORT_UART_PORTS; uart_idx++) {
		base = uart_mmap[uart_idx].base_pa;
		assert(base != 0);
		/* wakeup uart */
		sleep_req = mmio_read_32(UART_SLEEP_REQ_DRV(base));
		mmio_write_32(UART_SLEEP_REQ_DRV(base),
			sleep_req & (~MTK_UART_SEND_SLEEP_REQ));

		/* wait for UART to ACK */
		while (mmio_read_32(UART_SLEEP_ACK_DRV(base))
			& MTK_UART_SLEEP_ACK_IDLE) {
			if (i++ >= MTK_UART_WAIT_ACK_TIMES) {
				mmio_write_32(UART_SLEEP_REQ_DRV(base), sleep_req);
				ERROR("CANNOT GET UART%d WAKE ACK\n", uart_idx);
				return UART_PM_ERROR;
			}
			udelay(10);
		}
	}
	return 0;
}
