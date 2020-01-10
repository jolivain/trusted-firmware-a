/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2018-2020, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef LIMITS_H
#define LIMITS_H

#include <limits_.h>

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.1" "libc re-definition allowed") \
		(deviate "MISRA C-2012 Rule 21.2" "libc re-definition allowed")

#define CHAR_BIT   8
#define MB_LEN_MAX 1

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.1" "MISRA C-2012 Rule 21.2"

#endif /* LIMITS_H */
