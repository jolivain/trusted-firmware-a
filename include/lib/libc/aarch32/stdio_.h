/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STDIO__H
#define STDIO__H

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.1" "libc re-definition allowed") \
		(deviate "MISRA C-2012 Rule 21.2" "libc re-definition allowed")

#ifndef SSIZET_
typedef int ssize_t;
#define SSIZET_
#endif

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.1" "MISRA C-2012 Rule 21.2"

#endif /* STDIO__H */
