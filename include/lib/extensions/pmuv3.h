/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMUV3_H
#define PMUV3_H

#include <context.h>

#ifdef __aarch64__
void pmuv3_enable(cpu_context_t *ctx);
void pmuv3_enable_el2(void);
#else /* !__aarch64__ */
void pmuv3_disable_el3(void);
#endif /* !__aarch64__ */

#endif /* PMUV3_H */
