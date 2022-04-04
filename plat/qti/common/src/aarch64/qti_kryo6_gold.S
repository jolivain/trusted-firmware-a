/*
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
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
#include <cpu_macros.S>

#include <plat_macros.S>
#include <qti_cpu.h>

	.p2align 3

/* -------------------------------------------------
 * The CPU Ops reset function for Kryo-3 Gold
 * -------------------------------------------------
 */
func qti_kryo6_gold_reset_func
#if IMAGE_BL31 && WORKAROUND_CVE_2017_5715
	adr	x0, wa_cve_2017_5715_bpiall_vbar
	msr	vbar_el3, x0
	isb
#endif

	mov	x19, x30

	bl	qtiseclib_kryo6_gold_reset_asm
	mov	x30, x19
	b	cortex_a78_reset_func

endfunc qti_kryo6_gold_reset_func

/* ----------------------------------------------------
 * The CPU Ops core power down function for Kryo-3 Gold
 * ----------------------------------------------------
 */
func qti_kryo6_gold_core_pwr_dwn
	b cortex_a78_core_pwr_dwn
endfunc qti_kryo6_gold_core_pwr_dwn

/* -------------------------------------------------------
 * The CPU Ops cluster power down function for Kryo-3 Gold
 * -------------------------------------------------------
 */
func qti_kryo6_gold_cluster_pwr_dwn
	ret
endfunc qti_kryo6_gold_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Kryo4 Gold. Must follow AAPCS.
 */
func qti_kryo6_gold_errata_report
	/* TODO : Need to add support. Required only for debug bl31 image.*/
	ret
endfunc qti_kryo6_gold_errata_report
#endif

/* ---------------------------------------------
 * This function provides kryo4_gold specific
 * register information for crash reporting.
 * It needs to return with x6 pointing to
 * a list of register names in ASCII and
 * x8 - x15 having values of registers to be
 * reported.
 * ---------------------------------------------
 */
.section .rodata.qti_kryo4_gold_regs, "aS"
qti_kryo6_gold_regs:  /* The ASCII list of register names to be reported */
	.asciz	""

func qti_kryo6_gold_cpu_reg_dump
	adr	x6, qti_kryo6_gold_regs
	ret
endfunc qti_kryo6_gold_cpu_reg_dump

declare_cpu_ops	qti_kryo6_gold, QTI_KRYO6_GOLD_MIDR,	\
		qti_kryo6_gold_reset_func,		\
		qti_kryo6_gold_core_pwr_dwn,	\
		qti_kryo6_gold_cluster_pwr_dwn
