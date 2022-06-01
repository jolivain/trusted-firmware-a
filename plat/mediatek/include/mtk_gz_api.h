/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_GZ_API_H
#define MTK_GZ_API_H

#include <stdint.h>

uint32_t is_el2_enabled(void);
uint64_t get_el2_exec_start_offset(void);
uint64_t get_el2_reserved_mem_size(void);

#endif /* MTK_GZ_API_H */
