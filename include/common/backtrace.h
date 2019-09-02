/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <stdint.h>

uintptr_t demangle_address(uintptr_t addr, unsigned int el);

#endif /* BACKTRACE_H */
