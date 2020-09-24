/*
 * Copyright (c) 2019 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_USB_H
#define PLATFORM_USB_H

#include <platform_def.h>

#define USB3_DRD		BIT(0)
#define USB3H_USB2DRD		BIT(1)

extern const unsigned int xhc_portsc_reg_offset[MAX_USB_PORTS];

int xhci_phy_init(void);

#endif /* PLATFORM_USB_H */
