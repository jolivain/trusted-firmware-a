/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2018-2020, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef STDLIB_H
#define STDLIB_H

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.1" "libc re-definition allowed") \
		(deviate "MISRA C-2012 Rule 21.2" "libc re-definition allowed")

#include <stddef.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define _ATEXIT_MAX 1

extern void abort(void);
extern int atexit(void (*func)(void));
extern void exit(int status);

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.1" "MISRA C-2012 Rule 21.2"

#endif /* STDLIB_H */
