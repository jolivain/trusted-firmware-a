/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2018-2020, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef STDIO_H
#define STDIO_H

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.1" "libc re-definition allowed") \
		(deviate "MISRA C-2012 Rule 21.2" "libc re-definition allowed")

#include <cdefs.h>
#include <stddef.h>
#include <stdio_.h>

#define EOF            -1

int printf(const char *fmt, ...) __printflike(1, 2);
int snprintf(char *s, size_t n, const char *fmt, ...) __printflike(3, 4);

#ifdef STDARG_H
int vprintf(const char *fmt, va_list args);
#endif

int putchar(int c);
int puts(const char *s);

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.1" "MISRA C-2012 Rule 21.2"

#endif /* STDIO_H */
