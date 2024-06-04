/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_HELPERS_H
#define FVP_HELPERS_H

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stdbool.h>

#if __aarch64__
bool check_cpupwrctrl_el1_is_available(void);
#endif /* __aarch64__ */

#endif /* __ASSEMBLER__ */

#endif /* FVP_HELPERS_H */
