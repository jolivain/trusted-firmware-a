/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEMANGLE_H
#define DEMANGLE_H

#include <stdint.h>

#if AARCH64
uintptr_t demangle_address(uintptr_t addr, unsigned int el);
#endif

#endif /* DEMANGLE_H */
