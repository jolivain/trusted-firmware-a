/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2018-2020, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef STDDEF_H
#define STDDEF_H

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.1" "libc re-definition allowed") \
		(deviate "MISRA C-2012 Rule 21.2" "libc re-definition allowed")

#include <stddef_.h>

#ifndef _PTRDIFF_T
typedef long ptrdiff_t;
#define _PTRDIFF_T
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define offsetof(st, m) __builtin_offsetof(st, m)

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.1" "MISRA C-2012 Rule 21.2"

#endif /* STDDEF_H */
