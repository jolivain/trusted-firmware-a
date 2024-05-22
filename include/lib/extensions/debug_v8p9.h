/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEBUG_V8P9_H
#define DEBUG_V8P9_H

#if ENABLE_FEAT_DEBUGV8P9
void debugv8p9_extended_bp_wp_enable(void);
#else
static inline void debugv8p9_extended_bp_wp_enable(void)
{
}
#endif /* ENABLE_FEAT_DEBUGV8P9 */

#endif /* DEBUG_V8P9_H */
