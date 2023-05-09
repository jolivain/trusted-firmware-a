/*
 * Copyright (C) 2017-2023 Nuvoton Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ARBEL_CLOCK_H_
#define __ARBEL_CLOCK_H_

enum {
	APB1 = 1,
	APB2 = 2,
	APB3 = 3,
	APB4 = 4,
	APB5 = 5,
	SPI0 = 10,
	SPI3 = 13,
};

struct clk_ctl {
	unsigned int	clken1;
	unsigned int	clksel;
	unsigned int	clkdiv1;
	unsigned int	pllcon0;
	unsigned int	pllcon1;
	unsigned int	swrstr;
	unsigned char	res1[0x8];
	unsigned int	ipsrst1;
	unsigned int	ipsrst2;
	unsigned int	clken2;
	unsigned int	clkdiv2;
	unsigned int	clken3;
	unsigned int	ipsrst3;
	unsigned int	wd0rcr;
	unsigned int	wd1rcr;
	unsigned int	wd2rcr;
	unsigned int	swrstc1;
	unsigned int	swrstc2;
	unsigned int	swrstc3;
	unsigned int	tiprstc;
	unsigned int	pllcon2;
	unsigned int	clkdiv3;
	unsigned int	corstc;
	unsigned int	pllcong;
	unsigned int	ahbckfi;
	unsigned int	seccnt;
	unsigned int	cntr25m;
	unsigned int	clken4;
	unsigned int	ipsrst4;
	unsigned int	busto;
	unsigned int	clkdiv4;
	unsigned int	wd0rcrb;
	unsigned int	wd1rcrb;
	unsigned int	wd2rcrb;
	unsigned int	swrstc1b;
	unsigned int	swrstc2b;
	unsigned int	swrstc3b;
	unsigned int	tiprstcb;
	unsigned int	corstcb;
	unsigned int	ipsrstdis1;
	unsigned int	ipsrstdis2;
	unsigned int	ipsrstdis3;
	unsigned int	ipsrstdis4;
	unsigned char	res2[0x10];
	unsigned int	thrtl_cnt;
};

typedef enum arbel_reset_type {
	RESET_TYPE_NULL = 0,
	RESET_TYPE_VSB,
	RESET_TYPE_CORE,
	RESET_TYPE_CORE_OFF,
	RESET_TYPE_FSW,
	RESET_TYPE_WD0,
	RESET_TYPE_WD1,
	RESET_TYPE_WD2,
	RESET_TYPE_SW1,
	RESET_TYPE_SW2,
	RESET_TYPE_SW3,
	RESET_TYPE_TIP_MODULE,
	RESET_TYPE_TIP_PORST,
	RESET_TYPE_DBG_RESET,
	RESET_TYPE_PCI_RST,
	RESET_TYPE_LAST,
} arbel_reset_type;


#define CLK_BA					0xF0801000
/* Clock Enable 1 Register (CLKEN1) */
#define CLKEN1_TIMER0_4				19
#define CLKEN1_TIMER5_9				20
#define CLKEN1_EMC1				6

/* Clock Enable 2 Register (CLKEN2) */
#define CLKEN2_GMAC1				28
#define CLKEN2_USBIF				27
#define CLKEN2_USBH1				26
#define CLKEN2_GMAC2				25
#define CLKEN2_SDHC				9
#define CLKEN2_MMC				8

/* Clock Enable 2 Register (CLKEN4) */
#define CLKEN2_USBH2				31

/* Clock Select Register (CLKSEL) */
#define CLKSEL_RCPCKSEL				27
#define CLKSEL_RGSEL				25
#define CLKSEL_DVCSSEL				23
#define CLKSEL_GFXMSEL				21
#define CLKSEL_CLKOUTSEL			18
#define CLKSEL_GFXCKSEL				16
#define CLKSEL_TIMCKSEL				14
#define CLKSEL_MCCKSEL				12
#define CLKSEL_SUCKSEL				10
#define CLKSEL_UARTCKSEL			8
#define CLKSEL_SDCKSEL				6
#define CLKSEL_PIXCKSEL				4
#define CLKSEL_GPRFSEL				2
#define CLKSEL_CPUCKSEL				0

/* Clock Divider Control Register 1 (CLKDIV1) */
#define CLKDIV1_ADCCKDIV			28
#define CLKDIV1_CLK4DIV				26
#define CLKDIV1_TIMCKDIV			21
#define CLKDIV1_UARTDIV				16
#define CLKDIV1_MMCCKDIV			11
#define CLKDIV1_AHB3CKDIV			6
#define CLKDIV1_PCICKDIV			2
#define CLKDIV1_CLK2DIV				0

/* Clock Divider Control Register 2 (CLKDIV2) */
#define CLKDIV2_APB4CKDIV			30
#define CLKDIV2_APB3CKDIV			28
#define CLKDIV2_APB2CKDIV			26
#define CLKDIV2_APB1CKDIV			24
#define CLKDIV2_APB5CKDIV			22
#define CLKDIV2_CLKOUTDIV			16
#define CLKDIV2_GFXCKDIV			13
#define CLKDIV2_SUCKDIV				8
#define CLKDIV2_SU48CKDIV			4
#define CLKDIV2_SD1CKDIV			0

/* Clock Divider Control Register 3 (CLKDIV3) */
#define CLKDIV3_SPI0CKDV			6
#define CLKDIV3_SPIXCKDV			1

/* Clock Divider Control Register 4 (CLKDIV4) */
#define CLKDIV4_RGREFDIV			28
#define CLKDIV4_RCPREFDIV			12

/* PLL Control Register 2 (PLLCON2) */
#define PLLCONn_LOKI				31
#define PLLCONn_LOKS				30
#define PLLCONn_FBDV				16
#define PLLCONn_OTDV2				13
#define PLLCONn_PWDEN				12
#define PLLCONn_OTDV1				8
#define PLLCONn_INDV				0

/* CPUCKSEL (CPU/AMBA/MC Clock Source Select Bit) */
#define CLKSEL_CPUCKSEL_PLL0		0x00	/* 0 0: PLL0 clock*/
#define CLKSEL_CPUCKSEL_PLL1		0x01	/* 0 1: PLL1 clock */
#define CLKSEL_CPUCKSEL_CLKREF		0x02	/* 1 0: CLKREF input (25 MHz, default) */
#define CLKSEL_CPUCKSEL_SYSBPCK		0x03	/* 1 1: Bypass clock from pin SYSBPCK */

/* UARTCKSEL (Core and Host UART Clock Source Select Bit). */
#define CLKSEL_UARTCKSEL_PLL0		0x00	/* 0 0: PLL0    clock. */
#define CLKSEL_UARTCKSEL_PLL1		0x01	/* 0 1: PLL1    clock. */
#define CLKSEL_UARTCKSEL_CLKREF		0x02	/* 1 0: CLKREF  clock (25 MHz, default). */
#define CLKSEL_UARTCKSEL_PLL2		0x03	/* 1 1: PLL2    clock divided by 2. */

/* SDCKSEL (SDHC Clock Source Select Bit). */
#define CLKSEL_SDCKSEL_PLL0			0x00	/* 0 0: PLL0    clock.  */
#define CLKSEL_SDCKSEL_PLL1			0x01	/* 0 1: PLL1    clock.  */
#define CLKSEL_SDCKSEL_CLKREF		0x02	/* 1 0: CLKREF clock (25 MHz, default).  */
#define CLKSEL_SDCKSEL_PLL2			0x03	/* 1 1: PLL2    clock divided by 2.  */

/* TIMCKSEL (Timer Clock Source Select Bit) */
#define CLKSEL_TIMCKSEL_PLL0		0x00	/* 0 0: PLL0    clock.  */
#define CLKSEL_TIMCKSEL_PLL1		0x01	/* 0 1: PLL1    clock.  */
#define CLKSEL_TIMCKSEL_CLKREF		0x02	/* 1 0: CLKREF  clock (25 MHz, default).  */
#define CLKSEL_TIMCKSEL_PLL2		0x03	/* 1 1: PLL2    clock divided by 2.  */

/* IP Software Reset Register 1 (IPSRST1), offset 0x20 */
#define IPSRST1_USBDEV1				5
#define IPSRST1_USBDEV2				8
#define IPSRST1_USBDEV3				25
#define IPSRST1_USBDEV4				22
#define IPSRST1_USBDEV5				23
#define IPSRST1_USBDEV6				24
#define IPSRST1_GMAC4				21
#define IPSRST1_GMAC3				6

/* IP Software Reset Register 2 (IPSRST2), offset 0x24 */
#define IPSRST2_GMAC1				28
#define IPSRST2_GMAC2				25
#define IPSRST2_USBHOST1			26
#define IPSRST2_SDHC				9
#define IPSRST2_MMC					8

/* IP Software Reset Register 3 (IPSRST3), offset 0x34 */
#define IPSRST3_USBPHY1				24
#define IPSRST3_USBPHY2				25
#define IPSRST3_USBHUB				8
#define IPSRST3_USBDEV9				7
#define IPSRST3_USBDEV8				6
#define IPSRST3_USBDEV7				5
#define IPSRST3_USBDEV0				4

/* IP Software Reset Register 4 (IPSRST4), offset 0x74 */
#define IPSRST4_USBHOST2			31
#define IPSRST4_USBPHY3				25

#define SWRSTR					(CLK_BA+0x14)

/* SWRSTR (Software Reset Register) */
#define SWRSTR_SWRST1				0x08
#define SWRSTR_SWRST2				0x10
#define SWRSTR_SWRST3				0x20

/* Full system watchdog */

#define FSW_BA					0xF000F000
#define FSWCR						(FSW_BA + 0x1C)
#define FSWCR_WDT_CNT				23
#define FSWCR_WDTCK					10
#define FSWCR_WTE					7
#define FSWCR_WTIS					4
#define FSWCR_WTRE					1
#define FSWCR_WTR					0

void arbel_handle_reset(arbel_reset_type reset_t);
int arbel_clk_if_init();

#endif /* __ARBEL_CLOCK_H_ */
