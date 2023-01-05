/*
 * Copyright (c) 2013-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

__attribute__((weak)) int putchar(int c)
{
	return c;
}
