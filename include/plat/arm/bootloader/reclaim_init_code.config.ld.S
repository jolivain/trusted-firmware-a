/*
 * Copyright (c) 2017-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RECLAIM_INIT_CODE_CONFIG_LD_S
#define RECLAIM_INIT_CODE_CONFIG_LD_S

#define RECLAIM_INIT_CODE_SECTION .noinit.stacks
#define RECLAIM_INIT_CODE_SECTION_OFFSET PLATFORM_STACK_SIZE
#define RECLAIM_INIT_CODE_SECTION_REGION n
#define RECLAIM_INIT_CODE_SECTION_PHDR n

#endif /* RECLAIM_INIT_CODE_CONFIG_LD_S */
