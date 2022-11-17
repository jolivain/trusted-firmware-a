/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRBE_H
#define TRBE_H

#if ENABLE_TRBE_FOR_NS
void trbe_enable(void);
#else
static inline void trbe_enable(void)
{
}
#endif

#endif /* TRBE_H */
