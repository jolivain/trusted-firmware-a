/*
 * Copyright (c) 2022, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <platform_def.h>

        /* Declare function */
	.global asm_el3_uptime
        .global asm_print_dec
        .global mtk_print_timestamp

/* The offset to add to get ascii for numerals '0 - 9' */
#define ASCII_OFFSET_NUM        0x30

timestamp_msg:
        .asciz "Timestamp:\t0x"
print_dot:
        .asciz  "."

/*
 * This function calculate el3_uptime in x6.
 * Out: x6 = return el3_uptime.
 * Clobber: x2, x3, x30
 */
func asm_el3_uptime
        mrs     x2, cntpct_el0
        ldr     x3, =el3_time_base
        ldr     x3, [x3]
        sub     x2, x2, x3
        mov     x3, #SYS_COUNTER_FREQ_IN_MHZ
        udiv    x2, x2, x3
        ldr     x3, =align_time_base
        ldr     x3, [x3]
        sub     x2, x2, x3
        mov x6, x2

        ret     x30
endfunc asm_el3_uptime

/*
 * This function prints a decimal number in x4.
 * In: x4 = the decimal to print.
 * Clobber: x30, x0 - x3, x5
 */
func asm_print_dec
	mov     x3, x30
	mov     x0, x4
	mov     x5, #0
	cbz     x4, 1f
	/*
	 * This timestamp can shows boot up time
	 * around 3170979 years.
	 */
	ldr x5, =10000000000000000000
	mov x1, #10
3:
	udiv x5, x5, x1
	udiv    x0, x4, x5
	cbz     x0, 3b
1:
	add     x0, x0, #ASCII_OFFSET_NUM
	bl      plat_crash_console_putc
	udiv    x0, x4, x5
	msub    x4, x0, x5, x4
	mov x1, #10
	udiv x5, x5, x1
	cbz     x5, 2f
	udiv    x0, x4, x5
	b 1b
2:
	ret     x3
endfunc asm_print_dec

/*
 * This function prints MTK timestamp in hex and decimal.
 * Clobber: x30, x0 - x6, x16
 */
func mtk_print_timestamp
	mov x16, x30
        /* Get timestamp in x6*/
	bl asm_el3_uptime

	adr x4, timestamp_msg
        bl      asm_print_str
        mov     x4, x6
        bl      asm_print_hex

        mov     x0, ','
        bl      plat_crash_console_putc
        mov     x0, ' '
        bl      plat_crash_console_putc

        ldr x3, =1000000
        udiv    x4, x6, x3        /* sec_time = cur_time / el3_uptime unit */
        bl      asm_print_dec

        adr x4, print_dot
        bl      asm_print_str
        ldr x3, =1000000
        udiv    x4, x6, x3        /* sec_time = cur_time / el3_uptime unit */
        msub    x1, x4, x3, x6    /* ns_time = (cur_time % el3_uptime unit)/1000 */
        mov x3, #1000
        udiv    x1, x1, x3
        mov x4, x1
        bl      asm_print_dec

        /* Print done */
        bl      asm_print_newline
	ret	x16
endfunc mtk_print_timestamp
