/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CDEFS_H
#define CDEFS_H

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.1" "libc re-definition allowed") \
		(deviate "MISRA C-2012 Rule 21.2" "libc re-definition allowed")

#define __dead2		__attribute__((__noreturn__))
#define __deprecated	__attribute__((__deprecated__))
#define __packed	__attribute__((__packed__))
#define __used		__attribute__((__used__))
#define __unused	__attribute__((__unused__))
#define __aligned(x)	__attribute__((__aligned__(x)))
#define __section(x)	__attribute__((__section__(x)))
#if RECLAIM_INIT_CODE
/*
 * Add each function to a section that is unique so the functions can still
 * be garbage collected
 */
#define __init		__section(".text.init." __FILE__ "." __XSTRING(__LINE__))
#else
#define __init
#endif

#define __printflike(fmtarg, firstvararg) \
		__attribute__((__format__ (__printf__, fmtarg, firstvararg)))

#define __weak_reference(sym, alias)	\
	__asm__(".weak alias");		\
	__asm__(".equ alias, sym")

#define __STRING(x)	#x
#define __XSTRING(x)	__STRING(x)

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.1" "MISRA C-2012 Rule 21.2"

#endif /* CDEFS_H */
