/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BRBE_H
#define BRBE_H

#if ENABLE_BRBE_FOR_NS
void brbe_enable(void);
#else
static inline void brbe_enable(void)
{
}
#endif

#endif /* BRBE_H */
