/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FPGA_PRIVATE_H
#define FPGA_PRIVATE_H

#define C_RUNTIME_READY_KEY (0xaa55aa55)

#ifndef __ASSEMBLER__
unsigned int plat_fpga_calc_core_pos(u_register_t mpidr);

void fpga_console_init(void);

void plat_fpga_gic_init(void);
void fpga_pwr_gic_on_finish(void);
void fpga_pwr_gic_off(void);
unsigned int fpga_get_cpu_cluster(void);
#endif /* __ASSEMBLER__ */

#endif /* FPGA_PRIVATE_H */
