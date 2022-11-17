/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRF_H
#define TRF_H

#if ENABLE_TRF_FOR_NS
void trf_enable(void);
#else
static inline void trf_enable(void)
{
}
#endif

#endif /* TRF_H */
