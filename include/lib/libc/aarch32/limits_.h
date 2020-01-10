/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.1" "libc re-definition allowed") \
		(deviate "MISRA C-2012 Rule 21.2" "libc re-definition allowed")

#define SCHAR_MAX  0x7F
#define SCHAR_MIN  (-SCHAR_MIN - 1)
#define CHAR_MAX   0x7F
#define CHAR_MIN   (-CHAR_MAX - 1)
#define UCHAR_MAX  0xFFU
#define SHRT_MAX   0x7FFF
#define SHRT_MIN   (-SHRT_MAX - 1)
#define USHRT_MAX  0xFFFFU
#define INT_MAX    0x7FFFFFFF
#define INT_MIN    (-INT_MAX - 1)
#define UINT_MAX   0xFFFFFFFFU
#define LONG_MAX   0x7FFFFFFFL
#define LONG_MIN   (-LONG_MAX - 1L)
#define ULONG_MAX  0xFFFFFFFFUL
#define LLONG_MAX  0x7FFFFFFFFFFFFFFFLL
#define LLONG_MIN  (-LLONG_MAX - 1LL)
#define ULLONG_MAX 0xFFFFFFFFFFFFFFFFULL

#define __LONG_BIT 32
#define __WORD_BIT 32

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.1" "MISRA C-2012 Rule 21.2"
