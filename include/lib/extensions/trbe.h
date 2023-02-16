/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRBE_H
#define TRBE_H

#if ENABLE_TRBE_FOR_NS
void trbe_enable_el3(void);
void trbe_enable_el2(void);
#else
static inline void trbe_enable_el3(void)
{
}
static inline void trbe_enable_el2(void)
{
}
#endif /* ENABLE_TRBE_FOR_NS */

#endif /* TRBE_H */
