/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ASSERT_H
#define ASSERT_H

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.1" "libc re-definition allowed") \
		(deviate "MISRA C-2012 Rule 21.2" "libc re-definition allowed")

#include <cdefs.h>

#include <platform_def.h>

#include <common/debug.h>

#ifndef PLAT_LOG_LEVEL_ASSERT
#define PLAT_LOG_LEVEL_ASSERT	LOG_LEVEL
#endif

#if ENABLE_ASSERTIONS
# if PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_VERBOSE
#  define assert(e)	((e) ? (void)0 : __assert(__FILE__, __LINE__, #e))
# elif PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_INFO
#  define assert(e)	((e) ? (void)0 : __assert(__FILE__, __LINE__))
# else
#  define assert(e)	((e) ? (void)0 : __assert())
# endif
#else
#define assert(e)	((void)0)
#endif /* ENABLE_ASSERTIONS */

#if PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_VERBOSE
void __dead2 __assert(const char *file, unsigned int line,
		      const char *assertion);
#elif PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_INFO
void __dead2 __assert(const char *file, unsigned int line);
#else
void __dead2 __assert(void);
#endif

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.1" "MISRA C-2012 Rule 21.2"

#endif /* ASSERT_H */
