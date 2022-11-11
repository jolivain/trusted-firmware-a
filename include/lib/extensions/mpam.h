/*
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPAM_H
#define MPAM_H

#include <stdbool.h>

/*
 * mpam version information is needed within EL2 context management framework
 * and hence has to be declared as an extern.
 */
extern uint8_t mpam_version;

void mpam_enable(bool el2_unused);

#endif /* MPAM_H */
