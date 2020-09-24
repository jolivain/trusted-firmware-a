/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef USB_PHY_H
#define USB_PHY_H

#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define DRDU2_U2PLL_NDIV_FRAC_OFFSET            0x0

#define DRDU2_U2PLL_NDIV_INT                    0x4

#define DRDU2_U2PLL_CTRL                        0x8
#define DRDU2_U2PLL_LOCK                        BIT(6)
#define DRDU2_U2PLL_RESETB                      BIT(5)
#define DRDU2_U2PLL_PDIV_MASK                   0xF
#define DRDU2_U2PLL_PDIV_OFFSET                 1
#define DRDU2_U2PLL_SUSPEND_EN                  BIT(0)

#define DRDU2_PHY_CTRL                          0x0C
#define DRDU2_U2IDDQ                            BIT(30)
#define DRDU2_U2SOFT_RST_N                      BIT(29)
#define DRDU2_U2PHY_ON_FLAG                     BIT(22)
#define DRDU2_U2PHY_PCTL_MASK                   0xFFFF
#define DRDU2_U2PHY_PCTL_OFFSET                 6
#define DRDU2_U2PHY_RESETB                      BIT(5)
#define DRDU2_U2PHY_ISO                         BIT(4)
#define DRDU2_U2AFE_BG_PWRDWNB                  BIT(3)
#define DRDU2_U2AFE_PLL_PWRDWNB                 BIT(2)
#define DRDU2_U2AFE_LDO_PWRDWNB                 BIT(1)
#define DRDU2_U2CTRL_CORERDY                    BIT(0)

#define DRDU2_STRAP_CTRL                        0x18
#define DRDU2_FORCE_HOST_MODE                   BIT(5)
#define DRDU2_FORCE_DEVICE_MODE                 BIT(4)
#define BDC_USB_STP_SPD_MASK                    0x7
#define BDC_USB_STP_SPD_OFFSET                  0

#define DRDU2_PWR_CTRL                          0x1C
#define DRDU2_U2PHY_DFE_SWITCH_PWROKIN_I        BIT(2)
#define DRDU2_U2PHY_DFE_SWITCH_PWRONIN_I        BIT(1)

#define DRDU2_SOFT_RESET_CTRL                   0x20
#define DRDU2_BDC_AXI_SOFT_RST_N                BIT(0)

#define USB3H_U2PLL_NDIV_FRAC                   0x4

#define USB3H_U2PLL_NDIV_INT                    0x8

#define USB3H_U2PLL_CTRL                        0xC
#define USB3H_U2PLL_LOCK                        BIT(6)
#define USB3H_U2PLL_RESETB                      BIT(5)
#define USB3H_U2PLL_PDIV_MASK                   0xF
#define USB3H_U2PLL_PDIV_OFFSET                 1

#define USB3H_U2PHY_CTRL                        0x10
#define USB3H_U2PHY_ON_FLAG                     22
#define USB3H_U2PHY_PCTL_MASK                   0xFFFF
#define USB3H_U2PHY_PCTL_OFFSET                 6
#define USB3H_U2PHY_IDDQ                        BIT(29)
#define USB3H_U2PHY_RESETB                      BIT(5)
#define USB3H_U2PHY_ISO                         BIT(4)
#define USB3H_U2AFE_BG_PWRDWNB                  BIT(3)
#define USB3H_U2AFE_PLL_PWRDWNB                 BIT(2)
#define USB3H_U2AFE_LDO_PWRDWNB                 BIT(1)
#define USB3H_U2CTRL_CORERDY                    BIT(0)

#define USB3H_U3PHY_CTRL                        0x14
#define USB3H_U3SOFT_RST_N                      BIT(30)
#define USB3H_U3MDIO_RESETB_I                   BIT(29)
#define USB3H_U3POR_RESET_I                     BIT(28)
#define USB3H_U3PHY_PCTL_MASK                   0xFFFF
#define USB3H_U3PHY_PCTL_OFFSET                 2
#define USB3H_U3PHY_RESETB                      BIT(1)

#define USB3H_U3PHY_PLL_CTRL                    0x18
#define USB3H_U3PLL_REFCLK_MASK                 0x7
#define USB3H_U3PLL_REFCLK_OFFSET               4
#define USB3H_U3PLL_SS_LOCK                     BIT(3)
#define USB3H_U3PLL_SEQ_START                   BIT(2)
#define USB3H_U3SSPLL_SUSPEND_EN                BIT(1)
#define USB3H_U3PLL_RESETB                      BIT(0)

#define USB3H_PWR_CTRL                          0x28
#define USB3H_PWR_CTRL_OVERRIDE_I_R             4
#define USB3H_PWR_CTRL_U2PHY_DFE_SWITCH_PWROKIN BIT(11)
#define USB3H_PWR_CTRL_U2PHY_DFE_SWITCH_PWRONIN BIT(10)

#define USB3H_SOFT_RESET_CTRL                   0x2C
#define USB3H_XHC_AXI_SOFT_RST_N                BIT(1)

#define USB3H_PHY_PWR_CTRL                      0x38
#define USB3H_DISABLE_USB30_P0                  BIT(2)
#define USB3H_DISABLE_EUSB_P1                   BIT(1)
#define USB3H_DISABLE_EUSB_P0                   BIT(0)


#define DRDU3_U2PLL_NDIV_FRAC                   0x4

#define DRDU3_U2PLL_NDIV_INT                    0x8

#define DRDU3_U2PLL_CTRL                        0xC
#define DRDU3_U2PLL_LOCK                        BIT(6)
#define DRDU3_U2PLL_RESETB                      BIT(5)
#define DRDU3_U2PLL_PDIV_MASK                   0xF
#define DRDU3_U2PLL_PDIV_OFFSET                 1

#define DRDU3_U2PHY_CTRL                        0x10
#define DRDU3_U2PHY_IDDQ                        BIT(29)
#define DRDU3_U2PHY_ON_FLAG                     BIT(22)
#define DRDU3_U2PHY_PCTL_MASK                   0xFFFF
#define DRDU3_U2PHY_PCTL_OFFSET                 6
#define DRDU3_U2PHY_RESETB                      BIT(5)
#define DRDU3_U2PHY_ISO                         BIT(4)
#define DRDU3_U2AFE_BG_PWRDWNB                  BIT(3)
#define DRDU3_U2AFE_PLL_PWRDWNB                 BIT(2)
#define DRDU3_U2AFE_LDO_PWRDWNB                 BIT(1)
#define DRDU3_U2CTRL_CORERDY                    BIT(0)

#define DRDU3_U3PHY_CTRL                        0x14
#define DRDU3_U3XHC_SOFT_RST_N                  BIT(31)
#define DRDU3_U3BDC_SOFT_RST_N                  BIT(30)
#define DRDU3_U3MDIO_RESETB_I                   BIT(29)
#define DRDU3_U3POR_RESET_I                     BIT(28)
#define DRDU3_U3PHY_PCTL_MASK                   0xFFFF
#define DRDU3_U3PHY_PCTL_OFFSET                 2
#define DRDU3_U3PHY_RESETB                      BIT(1)

#define DRDU3_U3PHY_PLL_CTRL                    0x18
#define DRDU3_U3PLL_REFCLK_MASK                 0x7
#define DRDU3_U3PLL_REFCLK_OFFSET               4
#define DRDU3_U3PLL_SS_LOCK                     BIT(3)
#define DRDU3_U3PLL_SEQ_START                   BIT(2)
#define DRDU3_U3SSPLL_SUSPEND_EN                BIT(1)
#define DRDU3_U3PLL_RESETB                      BIT(0)

#define DRDU3_STRAP_CTRL                        0x28
#define BDC_USB_STP_SPD_MASK                    0x7
#define BDC_USB_STP_SPD_OFFSET                  0
#define BDC_USB_STP_SPD_SS                      0x0
#define BDC_USB_STP_SPD_HS                      0x2

#define DRDU3_PWR_CTRL                          0x2c
#define DRDU3_U2PHY_DFE_SWITCH_PWROKIN          BIT(12)
#define DRDU3_U2PHY_DFE_SWITCH_PWRONIN          BIT(11)
#define DRDU3_PWR_CTRL_OVERRIDE_I_R             4

#define DRDU3_SOFT_RESET_CTRL                   0x30
#define DRDU3_XHC_AXI_SOFT_RST_N                BIT(1)
#define DRDU3_BDC_AXI_SOFT_RST_N                BIT(0)

#define DRDU3_PHY_PWR_CTRL                      0x3c
#define DRDU3_DISABLE_USB30_P0                  BIT(2)
#define DRDU3_DISABLE_EUSB_P1                   BIT(1)
#define DRDU3_DISABLE_EUSB_P0                   BIT(0)

#define PLL_REFCLK_PAD                          0x0
#define PLL_REFCLK_25MHZ                        0x1
#define PLL_REFCLK_96MHZ                        0x2
#define PLL_REFCLK_INTERNAL                     0x3
/* USB PLL lock time out for 10 ms */
#define PLL_LOCK_RETRY_COUNT                    10000


#define U2PLL_NDIV_INT_VAL                      0x13
#define U2PLL_NDIV_FRAC_VAL                     0x1005
#define U2PLL_PDIV_VAL                          0x1
/*
 * Using external FSM
 * BIT-3:2: device mode; mode is not effect
 * BIT-1: soft reset active low
 */
#define U2PHY_PCTL_VAL                          0x0003
/* Non-driving signal low */
#define U2PHY_PCTL_NON_DRV_LOW                  0x0002
#define U3PHY_PCTL_VAL                          0x0006

#define MAX_NR_PORTS                            3

#define USB3H_DRDU2_PHY                         1
#define DRDU3_PHY                               2

#define USB_HOST_MODE                           1
#define USB_DEV_MODE                            2

#define USB3SS_PORT                             0
#define DRDU2_PORT                              1
#define USB3HS_PORT                             2

#define DRD3SS_PORT                             0
#define DRD3HS_PORT                             1

#define SR_USB_PHY_COUNT                        2

#define DRDU3_PIPE_CTRL			0x68500000
#define DRDU3H_XHC_REGS_CPLIVER		0x68501000
#define USB3H_PIPE_CTRL			0x68510000
#define DRD2U3H_XHC_REGS_CPLIVER	0x68511000
#define DRDU2_U2PLL_NDIV_FRAC		0x68520000

#define AXI_DEBUG_CTRL				0x68500038
#define AXI_DBG_CTRL_SSPHY_DRD_MODE_DISABLE	BIT(12)

#define USB3H_DEBUG_CTRL			0x68510034
#define USB3H_DBG_CTRL_SSPHY_DRD_MODE_DISABLE	BIT(7)

typedef struct _usb_phy_port usb_phy_port_t;

typedef struct {
	uint32_t drdu2reg;
	uint32_t usb3hreg;
	uint32_t drdu3reg;
	uint32_t phy_id;
	uint32_t ports_enabled;
	uint32_t initialized;
	usb_phy_port_t *phy_port;
} usb_phy_t;

struct _usb_phy_port {
	uint32_t port_id;
	uint32_t mode;
	uint32_t enabled;
	usb_phy_t *p;
};

struct u2_phy_ext_fsm {
	uint32_t pll_ctrl_reg;
	uint32_t phy_ctrl_reg;
	uint32_t phy_iddq;
	uint32_t pwr_ctrl_reg;
	uint32_t pwr_okin;
	uint32_t pwr_onin;
};

#endif /* USB_PHY_H */
