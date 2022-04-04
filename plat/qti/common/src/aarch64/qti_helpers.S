/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
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

#include <arch.h>
#include <asm_macros.S>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/gicv3.h>
#include <drivers/console.h>

#include <platform_def.h>

	.globl	plat_my_core_pos
	.globl	plat_qti_core_pos_by_mpidr
	.globl	plat_reset_handler
	.globl	plat_panic_handler

	/* -----------------------------------------------------
	 *  unsigned int plat_qti_core_pos_by_mpidr(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function:
	 *  CorePos = (ClusterId * 4) + CoreId
	 *  - In ARM v8   (MPIDR_EL1[24]=0)
	 *    ClusterId = MPIDR_EL1[15:8]
	 *    CoreId    = MPIDR_EL1[7:0]
	 *  - In ARM v8.1 (MPIDR_EL1[24]=1)
	 *    ClusterId = MPIDR_EL1[23:15]
	 *    CoreId    = MPIDR_EL1[15:8]
	 *  Clobbers: x0 & x1.
	 * -----------------------------------------------------
	 */
func plat_qti_core_pos_by_mpidr
	mrs	x1, mpidr_el1
	tst	x1, #MPIDR_MT_MASK
	beq	plat_qti_core_pos_by_mpidr_no_mt
	/* Right shift mpidr by one affinity level when MT=1. */
	lsr	x0, x0, #MPIDR_AFFINITY_BITS
plat_qti_core_pos_by_mpidr_no_mt:
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_qti_core_pos_by_mpidr

	/* --------------------------------------------------------------------
	 * void plat_panic_handler(void)
	 * calls SDI and reset system
	 * --------------------------------------------------------------------
	 */
func plat_panic_handler
	msr	spsel, #0
	bl	plat_set_my_stack
	b	qtiseclib_panic
endfunc plat_panic_handler

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_qti_calc_core_pos()
	 *  definition to get the index of the calling CPU
	 *  Clobbers: x0 & x1.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_qti_core_pos_by_mpidr
endfunc plat_my_core_pos

func plat_reset_handler
	/* save the lr */
	mov	x18, x30

	/* pass cold boot status. */
	ldr	w0, g_qti_bl31_cold_booted
	/* Execuete CPUSS boot set up on every core. */
	bl	qtiseclib_cpuss_reset_asm

	ret	x18
endfunc plat_reset_handler
