/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCU_H
#define SCU_H

void disable_scu(uintptr_t mpidr);
void enable_scu(uintptr_t mpidr);

#endif /* SCU_H */
