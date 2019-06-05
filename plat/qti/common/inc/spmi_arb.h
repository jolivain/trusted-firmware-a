/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __QTI_SPMI_ARB_H__
#define __QTI_SPMI_ARB_H__

#include <stdint.h>

/* 32-bit addresses combine (U)SID, PID and register address. */

int spmi_arb_read8(uint32_t addr);
int spmi_arb_write8(uint32_t addr, uint8_t data);

#endif /* __QTI_SPMI_ARB_H__ */
