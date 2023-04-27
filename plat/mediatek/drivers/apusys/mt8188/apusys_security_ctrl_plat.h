/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_SECURITY_CTRL_PLAT_H
#define APUSYS_SECURITY_CTRL_PLAT_H

#include <platform_def.h>

#define SOC2APU_SET1_0	(APU_SEC_CON + 0x0c)
#define SOC2APU_SET1_1	(APU_SEC_CON + 0x10)

#define REG_DOMAIN_NUM		(8)
#define REG_DOMAIN_BITS		(4)
#define DOMAIN_REMAP_SEL	BIT(6)

void apusys_security_ctrl_init(void);

#endif
