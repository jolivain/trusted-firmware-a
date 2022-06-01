/*
 * Copyright (c) 2021, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UART_DEF_H
#define UART_DEF_H

#include <stdint.h>

/*used in Task or normal function*/
#define UART_RBR_DRV(_baseaddr)			(_baseaddr+0x0)      /* Read only */
#define UART_THR_DRV(_baseaddr)			(_baseaddr+0x0)      /* Write only */
#define UART_IER_DRV(_baseaddr)			(_baseaddr+0x4)      /* Read Write */
#define UART_IIR_DRV(_baseaddr)			(_baseaddr+0x8)      /* Read only */
#define UART_FCR_DRV(_baseaddr)			(_baseaddr+0x8)      /* Write only */
#define UART_LCR_DRV(_baseaddr)			(_baseaddr+0xc)
#define UART_MCR_DRV(_baseaddr)			(_baseaddr+0x10)
#define UART_LSR_DRV(_baseaddr)			(_baseaddr+0x14)
#define UART_MSR_DRV(_baseaddr)			(_baseaddr+0x18)
#define UART_SCR_DRV(_baseaddr)			(_baseaddr+0x1c)
#define UART_DLL_DRV(_baseaddr)			(_baseaddr+0x0)
#define UART_DLH_DRV(_baseaddr)			(_baseaddr+0x4)
#define UART_AUTOBAUD_DRV(_baseaddr)		(_baseaddr+0x20)
#define UART_HIGHSPEED_DRV(_baseaddr)		(_baseaddr+0x24)
#define UART_SAMPLE_COUNT_DRV(_baseaddr)		(_baseaddr+0x28)
#define UART_SAMPLE_POINT_DRV(_baseaddr)		(_baseaddr+0x2c)
#define UART_AUTOBAUD_REG_DRV(_baseaddr)		(_baseaddr+0x30)
#define UART_RATE_FIX_REG_DRV(_baseaddr)		(_baseaddr+0x34)
#define UART_AUTO_BAUDSAMPLE_DRV(_baseaddr)		(_baseaddr+0x38)
#define UART_GUARD_DRV(_baseaddr)			(_baseaddr+0x3c)
#define UART_ESCAPE_DAT_DRV(_baseaddr)		(_baseaddr+0x40)
#define UART_ESCAPE_EN_DRV(_baseaddr)		(_baseaddr+0x44)
#define UART_SLEEP_EN_DRV(_baseaddr)		(_baseaddr+0x48)
#define UART_DMA_EN_DRV(_baseaddr)			(_baseaddr+0x4c)
#define UART_RXTRI_AD_DRV(_baseaddr)		(_baseaddr+0x50)
#define UART_FRACDIV_L_DRV(_baseaddr)		(_baseaddr+0x54)
#define UART_FRACDIV_M_DRV(_baseaddr)		(_baseaddr+0x58)
#define UART_FCR_RD_DRV(_baseaddr)			(_baseaddr+0x5C)
#define UART_USB_RX_SEL_DRV(_baseaddr)		(_baseaddr+0xB0)
#define UART_SLEEP_REQ_DRV(_baseaddr)		(_baseaddr+0xB4)
#define UART_SLEEP_ACK_DRV(_baseaddr)		(_baseaddr+0xB8)
#define UART_SPM_SEL_DRV(_baseaddr)			(_baseaddr+0xBC)
#define UART_EFR(_baseaddr)			(_baseaddr+0x98)      /* Only when LCR = 0xbf */
#define UART_XON1(_baseaddr)			(_baseaddr+0x10)     /* Only when LCR = 0xbf */
#define UART_XON2(_baseaddr)			(_baseaddr+0x14)     /* Only when LCR = 0xbf */
#define UART_XOFF1(_baseaddr)			(_baseaddr+0x18)     /* Only when LCR = 0xbf */
#define UART_XOFF2(_baseaddr)			(_baseaddr+0x1c)     /* Only when LCR = 0xbf */

#define UART_FEATURE_SEL(_baseaddr)		(_baseaddr+0x9c) /* Only when FEATURE_SEL= 0x1 */
#define UART_DLL_NEW(_baseaddr)			(_baseaddr+0x90) /* Only when FEATURE_SEL= 0x1 */
#define UART_DLH_NEW(_baseaddr)			(_baseaddr+0x94) /* Only when FEATURE_SEL= 0x1 */
#define UART_NEW_MAP_ENABLE   0x1

#define MTK_UART_SEND_SLEEP_REQ	0x1	/* Request uart to sleep */
#define MTK_UART_SLEEP_ACK_IDLE	0x1	/* uart in idle state */
#define MTK_UART_WAIT_ACK_TIMES	50

/*UART error code*/
#define UART_DONE  U(0)
#define UART_PM_ERROR  U(1)

struct mtk_uart_register {
	uint32_t dll;
	uint32_t dlh;
	uint32_t ier;
	uint32_t lcr;
	uint32_t mcr;
	uint32_t scr;
	uint32_t fcr;
	uint32_t lsr;
	uint32_t efr;
	uint32_t highspeed;
	uint32_t sample_count;
	uint32_t sample_point;
	uint32_t fracdiv_l;
	uint32_t fracdiv_m;
	uint32_t escape_en;
	uint32_t guard;
	uint32_t rx_sel;
};

struct mtk_uart {
	uint32_t base;
	struct mtk_uart_register registers;
};

#endif
