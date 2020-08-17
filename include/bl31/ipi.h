/*
 * Copyright (c) 2020 Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPI_H
#define IPI_H

#ifndef __ASSEMBLER__

#include <cdefs.h>
#include <stdint.h>

void ipi_send_cpu_stop(u_register_t target);
void ipi_init(void);

#endif /* __ASSEMBLER__ */

#endif /* IPI_H*/
