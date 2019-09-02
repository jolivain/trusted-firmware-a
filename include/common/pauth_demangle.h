/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PAUTH_DEMANGLE_H
#define PAUTH_DEMANGLE_H

#include <stdint.h>

#ifdef __aarch64__
uintptr_t demangle_address(uintptr_t addr, unsigned int el);
#endif

#endif /* PAUTH_DEMANGLE_H */
