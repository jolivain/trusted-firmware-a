/*
 * Copyright (c) 2020, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

	/* ---------------------------------------------
	 * The below macro prints out relevant GIC and
	 * CCI registers whenever an unhandled exception
	 * is taken in BL31.
	 * Clobbers: x0 - x10, x26, x27, sp
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs
	.endm
