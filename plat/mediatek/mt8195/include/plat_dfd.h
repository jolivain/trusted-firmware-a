/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_DFD_H
#define PLAT_DFD_H

extern void dfd_resume(void);
extern uint64_t dfd_smc_dispatcher(uint64_t arg0, uint64_t arg1,
					uint64_t arg2, uint64_t arg3);

#endif /* PLAT_DFD_H */
