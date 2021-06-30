/*
 * Copyright (c) 2013-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_R_ARCH_HELPERS_H
#define FVP_R_ARCH_HELPERS_H

#include <arch_helpers.h>
#include <arch.h>

/* v8-R64 MPU registers */
DEFINE_RENAME_SYSREG_RW_FUNCS(mpuir_el2, MPUIR_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(prenr_el2, PRENR_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(prselr_el2, PRSELR_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(prbar_el2, PRBAR_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(prlar_el2, PRLAR_EL2)

#endif /* FVP_R_ARCH_HELPERS_H */
