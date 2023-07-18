/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPE_H
#define SPE_H

#include <stdbool.h>

typedef struct spe_ctx {
	u_register_t pmblimitr_el1;
} spe_ctx_t;

#if ENABLE_SPE_FOR_NS
void spe_init_el3(void);
void spe_init_el2_unused(void);
void spe_disable(void);
spe_ctx_t spe_context_save(void);
void spe_context_restore(spe_ctx_t *spe_ctx);
#else
static inline void spe_init_el3(void)
{
}
static inline void spe_init_el2_unused(void)
{
}
static inline void spe_disable(void)
{
}
#endif /* ENABLE_SPE_FOR_NS */

#endif /* SPE_H */
