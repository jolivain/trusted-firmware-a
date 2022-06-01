/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#ifndef MTK_HELPERS_H
#define MTK_HELPERS_H

u_register_t plat_mpidr_by_core_pos(uint32_t core_id);
void *mtk_bl31_ramdump(const void *arg);
#endif /* MTK_HELPERS_H */
