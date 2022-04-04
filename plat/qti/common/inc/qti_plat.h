/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Changes from Qualcomm Innovation Center are provided under the following license:
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of Qualcomm Innovation Center, Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

#ifndef QTI_PLAT_H
#define QTI_PLAT_H

#include <stdint.h>

#include <common/bl_common.h>
#include <lib/cassert.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

/*
 * Utility functions common to QTI platforms
 */
int qti_mmap_add_dynamic_region(uintptr_t base_pa, size_t size,
				unsigned int attr);
int qti_mmap_remove_dynamic_region(uintptr_t base_va, size_t size);

/*
 * Utility functions common to ARM standard platforms
 */
void qti_setup_page_tables(
			   uintptr_t total_base,
			   size_t total_size,
			   uintptr_t code_start,
			   uintptr_t code_limit,
			   uintptr_t rodata_start,
			   uintptr_t rodata_limit
			  );

/*
 * Mandatory functions required in ARM standard platforms
 */
void plat_qti_gic_driver_init(void);
void plat_qti_gic_init(void);
void plat_qti_gic_cpuif_enable(void);
void plat_qti_gic_cpuif_disable(void);
void plat_qti_gic_pcpu_init(void);

/*
 * Optional functions required in ARM standard platforms
 */
unsigned int plat_qti_core_pos_by_mpidr(u_register_t mpidr);
unsigned int plat_qti_my_cluster_pos(void);

void gic_set_spi_routing(unsigned int id, unsigned int irm, u_register_t mpidr);

void qti_pmic_prepare_reset(void);
void qti_pmic_prepare_shutdown(void);

#endif /* QTI_PLAT_H */
