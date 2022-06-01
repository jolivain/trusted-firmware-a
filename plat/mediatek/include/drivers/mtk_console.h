/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_CONSOLE_H
#define MTK_CONSOLE_H

/* Console utility functions */
void mtk_console_core_init(void);
void mtk_console_core_end(void);

void mtk_mem_console_init(void *debug_buf_instance);
void mtk_mem_crash_console_init(void *debug_buf_instance);

#endif /* MTK_CONSOLE_H */
