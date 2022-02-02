/*
 * Copyright (c) 2018-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPAM_H
#define MPAM_H

#include <stdbool.h>

#include <context.h>

void mpam_enable(bool el2_unused, cpu_context_t *ctx);

#endif /* MPAM_H */
