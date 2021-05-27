/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPMM_H
#define MPMM_H

#include <stdbool.h>

/*
 * Enable the Maximum Power Mitigation Mechanism.
 *
 * This function will enable MPMM for the current core. The AMU counters
 * representing the MPMM gears must have been configured and enabled prior to
 * calling this function.
 */
void mpmm_enable(void);

/*
 * Re-enable MPMM after a cold reset of the core.
 */
void mpmm_context_restore(void);

#endif /* MPMM_H */
