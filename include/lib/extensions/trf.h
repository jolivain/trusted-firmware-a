/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRF_H
#define TRF_H

#include <context.h>

#if ENABLE_TRF_FOR_NS
void trf_enable(cpu_context_t *ctx);
void trf_enable_el2(void);
#else
static inline void trf_enable(cpu_context_t *ctx)
{
}
static inline void trf_enable_el2(void)
{
}
#endif /* ENABLE_TRF_FOR_NS */

#endif /* TRF_H */
