/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TWEDE_H
#define TWEDE_H

#include <stdint.h>

#define TWED_DISABLED UL(0xFFFFFFFF)

uint32_t plat_arm_set_twedel_scr_el3(void);
uint32_t plat_arm_set_twedel_hcr_el2(void);
uint32_t plat_arm_set_twedel_sctlr_elx(void);

#endif /* TWEDE_H */
