/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FGT2_H
#define FGT2_H

#include <context.h>

#if ENABLE_FEAT_FGT2
#if __aarch64__
void fgt2_enable(cpu_context_t *ctx);
#else
static inline void fgt2_enable(cpu_context_t *ctx)
{
}
#endif /* __aarch64__ */
#endif /* ENABLE_FEAT_FGT2 */

#endif /* FGT2_H */
