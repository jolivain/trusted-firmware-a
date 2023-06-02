/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRF_H
#define TRF_H

#include <context.h>

#if ENABLE_TRF_FOR_NS
#if __aarch64__
void trf_enable(cpu_context_t *ctx);
void trf_enable_el2(void);
#else /* !__aarch64__ */
void trf_enable_el3(void);
#endif /* __aarch64__ */
#else
#if __aarch64__
static inline void trf_enable(cpu_context_t *ctx)
{
}
static inline void trf_enable_el2(void)
{
}
#else /* !__aarch64__ */
static inline void trf_enable_el3(void)
{
}
#endif /* __aarch64__ */
#endif /* ENABLE_TRF_FOR_NS */

#endif /* TRF_H */
