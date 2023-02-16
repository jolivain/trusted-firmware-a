/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPAM_H
#define MPAM_H

#include <stdbool.h>

#if ENABLE_MPAM_FOR_LOWER_ELS
void mpam_enable_el3(void);
void mpam_enable_el2(void);
#else
static inline void mpam_enable_el3(void)
{
}
static inline void mpam_enable_el2(void)
{
}
#endif

#endif /* MPAM_H */
