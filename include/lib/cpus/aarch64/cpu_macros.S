/*
 * Copyright (c) 2014-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CPU_MACROS_S
#define CPU_MACROS_S

#include <assert_macros.S>
#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>

	/*
	 * Write given expressions as quad words
	 *
	 * _count:
	 *	Write at least _count quad words. If the given number of
	 *	expressions is less than _count, repeat the last expression to
	 *	fill _count quad words in total
	 * _rest:
	 *	Optional list of expressions. _this is for parameter extraction
	 *	only, and has no significance to the caller
	 *
	 * Invoked as:
	 *	fill_constants 2, foo, bar, blah, ...
	 */
	.macro fill_constants _count:req, _this, _rest:vararg
	  .ifgt \_count
	    /* Write the current expression */
	    .ifb \_this
	      .error "Nothing to fill"
	    .endif
	    .quad \_this

	    /* Invoke recursively for remaining expressions */
	    .ifnb \_rest
	      fill_constants \_count-1, \_rest
	    .else
	      fill_constants \_count-1, \_this
	    .endif
	  .endif
	.endm

	/*
	 * Declare CPU operations
	 *
	 * _name:
	 *	Name of the CPU for which operations are being specified
	 * _midr:
	 *	Numeric value expected to read from CPU's MIDR
	 * _resetfunc:
	 *	Reset function for the CPU. If there's no CPU reset function,
	 *	specify CPU_NO_RESET_FUNC
	 * _extra1:
	 *	This is a placeholder for future per CPU operations.  Currently,
	 *	some CPUs use this entry to set a test function to determine if
	 *	the workaround for CVE-2017-5715 needs to be applied or not.
	 * _extra2:
	 *	This is a placeholder for future per CPU operations. Currently
	 *	some CPUs use this entry to set a function to disable the
	 *	workaround for CVE-2018-3639.
	 * _extra3:
	 *	This is a placeholder for future per CPU operations. Currently,
	 *	some CPUs use this entry to set a test function to determine if
	 *	the workaround for CVE-2022-23960 needs to be applied or not.
	 * _e_handler:
	 *	This is a placeholder for future per CPU exception handlers.
	 * _power_down_ops:
	 *	Comma-separated list of functions to perform power-down
	 *	operatios on the CPU. At least one, and up to
	 *	CPU_MAX_PWR_DWN_OPS number of functions may be specified.
	 *	Starting at power level 0, these functions shall handle power
	 *	down at subsequent power levels. If there aren't exactly
	 *	CPU_MAX_PWR_DWN_OPS functions, the last specified one will be
	 *	used to handle power down at subsequent levels
	 */
	.macro declare_cpu_ops_base _name:req, _midr:req, _resetfunc:req, \
		_extra1:req, _extra2:req, _extra3:req, _e_handler:req, _power_down_ops:vararg
	.section cpu_ops, "a"
	.align 3
	.type cpu_ops_\_name, %object
	.quad \_midr
#if defined(IMAGE_AT_EL3)
	.quad \_resetfunc
#endif
	.quad \_extra1
	.quad \_extra2
	.quad \_extra3
	.quad \_e_handler
#ifdef IMAGE_BL31
	/* Insert list of functions */
	fill_constants CPU_MAX_PWR_DWN_OPS, \_power_down_ops
#endif
	/*
	 * It is possible (although unlikely) that a cpu may have no errata in
	 * code. In that case the start label will not be defined. The list is
	 * inteded to be used in a loop, so define it as zero-length for
	 * predictable behaviour. Since this macro is always called at the end
	 * of the cpu file (after all errata have been parsed) we can be sure
	 * that we are at the end of the list. Used by the errata ABI
	 */
	.pushsection .rodata.errata_entries
	.ifndef \_name\()_errata_list_start
		\_name\()_errata_list_start:
	.endif
	\_name\()_errata_list_end:
	.popsection

	/* and now put them in cpu_ops */
	.quad \_name\()_errata_list_start
	.quad \_name\()_errata_list_end

#if REPORT_ERRATA
	.ifndef \_name\()_cpu_str
	  /*
	   * Place errata reported flag, and the spinlock to arbitrate access to
	   * it in the data section.
	   */
	  .pushsection .data
	  define_asm_spinlock \_name\()_errata_lock
	  \_name\()_errata_reported:
	  .word	0
	  .popsection

	  /* Place CPU string in rodata */
	  .pushsection .rodata
	  \_name\()_cpu_str:
	  .asciz "\_name"
	  .popsection
	.endif


	/*
	 * Mandatory errata status printing function for CPUs of
	 * this class.
	 */
	.quad \_name\()_errata_report
	.quad \_name\()_cpu_str

#ifdef IMAGE_BL31
	/* Pointers to errata lock and reported flag */
	.quad \_name\()_errata_lock
	.quad \_name\()_errata_reported
#endif /* IMAGE_BL31 */
#endif /* REPORT_ERRATA */

#if defined(IMAGE_BL31) && CRASH_REPORTING
	.quad \_name\()_cpu_reg_dump
#endif
	.endm

	.macro declare_cpu_ops _name:req, _midr:req, _resetfunc:req, \
		_power_down_ops:vararg
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, 0, 0, 0, 0, \
			\_power_down_ops
	.endm

	.macro declare_cpu_ops_eh _name:req, _midr:req, _resetfunc:req, \
		_e_handler:req, _power_down_ops:vararg
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, \
			0, 0, 0, \_e_handler, \_power_down_ops
	.endm

	.macro declare_cpu_ops_wa _name:req, _midr:req, \
		_resetfunc:req, _extra1:req, _extra2:req, \
		_extra3:req, _power_down_ops:vararg
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, \
			\_extra1, \_extra2, \_extra3, 0, \_power_down_ops
	.endm

/* TODO can be deleted once all CPUs have been converted */
#if REPORT_ERRATA
	/*
	 * Print status of a CPU errata
	 *
	 * _chosen:
	 *	Identifier indicating whether or not a CPU errata has been
	 *	compiled in.
	 * _cpu:
	 *	Name of the CPU
	 * _id:
	 *	Errata identifier
	 * _rev_var:
	 *	Register containing the combined value CPU revision and variant
	 *	- typically the return value of cpu_get_rev_var
	 */
	.macro report_errata _chosen, _cpu, _id, _rev_var=x8
	/* Stash a string with errata ID */
	.pushsection .rodata
	\_cpu\()_errata_\_id\()_str:
	.asciz	"\_id"
	.popsection

	/* Check whether errata applies */
	mov	x0, \_rev_var
	/* Shall clobber: x0-x7 */
	bl	check_errata_\_id

	.ifeq \_chosen
	/*
	 * Errata workaround has not been compiled in. If the errata would have
	 * applied had it been compiled in, print its status as missing.
	 */
	cbz	x0, 900f
	mov	x0, #ERRATA_MISSING
	.endif
900:
	adr	x1, \_cpu\()_cpu_str
	adr	x2, \_cpu\()_errata_\_id\()_str
	bl	errata_print_msg
	.endm
#endif

	/*
	 * This macro is used on some CPUs to detect if they are vulnerable
	 * to CVE-2017-5715.
	 */
	.macro	cpu_check_csv2 _reg _label
	mrs	\_reg, id_aa64pfr0_el1
	ubfx	\_reg, \_reg, #ID_AA64PFR0_CSV2_SHIFT, #ID_AA64PFR0_CSV2_LENGTH
	/*
	 * If the field equals 1, branch targets trained in one context cannot
	 * affect speculative execution in a different context.
	 *
	 * If the field equals 2, it means that the system is also aware of
	 * SCXTNUM_ELx register contexts. We aren't using them in the TF, so we
	 * expect users of the registers to do the right thing.
	 *
	 * Only apply mitigations if the value of this field is 0.
	 */
#if ENABLE_ASSERTIONS
	cmp	\_reg, #3 /* Only values 0 to 2 are expected */
	ASM_ASSERT(lo)
#endif

	cmp	\_reg, #0
	bne	\_label
	.endm

	/*
	 * Helper macro that reads the part number of the current
	 * CPU and jumps to the given label if it matches the CPU
	 * MIDR provided.
	 *
	 * Clobbers x0.
	 */
	.macro  jump_if_cpu_midr _cpu_midr, _label
	mrs	x0, midr_el1
	ubfx	x0, x0, MIDR_PN_SHIFT, #12
	cmp	w0, #((\_cpu_midr >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	\_label
	.endm


// TODO what's the best way to do this? I think this is okish?
.equ ERRATA_WA_OFF, 0
.equ ERRATA_CHOSEN_OFF, 8
.equ ERRATA_ENTRY_SIZE, 24

/*
 * NOTE an erratum and CVE id could clash. However, both numbers are very large
 * and the probablity is minuscule. Working around this makes code very
 * complicated and extremely difficult to read so it is not considered. In the
 * unlikely event that this does happen, prepending the CVE id with a 0 should
 * resolve the conflict
 */
.macro add_erratum_entry _cpu:req, _cve:req, _id:req, _chosen:req, _apply_at_reset:req
	.pushsection .rodata.errata_entries
		.align	3
		.ifndef \_cpu\()_errata_list_start
		\_cpu\()_errata_list_start:
		.endif

		/* check if unused and compile out if no references */
		.if \_apply_at_reset && \_chosen
			.quad	erratum_\_cpu\()_\_id\()_wa
		.else
			.quad	0
		.endif
		/* TODO(errata ABI): this prevents all checker functions from
		 * being optimised away. Can be done away with unless the ABI
		 * needs them */
		.quad	check_erratum_\_cpu\()_\_id
		/* Will fit CVEs with up to 10 character in the ID field */
		.word	\_id
		.hword	\_cve
		.byte	\_chosen
		/* TODO(errata ABI): mitigated field for known but unmitigated
		 * errata*/
		.byte	0x1
	.popsection
.endm

.macro _workaround_start _cpu:req, _cve:req, _id:req, _chosen:req, _apply_at_reset:req
	add_erratum_entry \_cpu, \_cve, \_id, \_chosen, \_apply_at_reset

	func erratum_\_cpu\()_\_id\()_wa
		mov	x8, x30

		/* save rev_var for workarounds that might need it but don't
		 * restore to x0 because few will care */
		mov	x5, x0
		bl	check_erratum_\_cpu\()_\_id
		cbz	x0, erratum_\_cpu\()_\_id\()_skip
.endm

.macro _workaround_end _cpu:req, _id:req
	erratum_\_cpu\()_\_id\()_skip:
		ret	x8
	endfunc erratum_\_cpu\()_\_id\()_wa
.endm

/*******************************************************************************
 * Errata workaround wrappers
 ******************************************************************************/
.macro workaround_reset_start _cpu:req, _cve:req, _id:req, _chosen:req
	_workaround_start \_cpu, \_cve, \_id, \_chosen, 1
.endm

.macro workaround_runtime_start _cpu:req, _cve:req, _id:req, _chosen:req, _midr
	/*
	 * Let errata specify if they need MIDR checking. Sadly, storing the
	 * MIDR in an .equ to retrieve automatically blows up as it stores some
	 * brackets in the symbol
	 */
	.ifnb \_midr
		jump_if_cpu_midr \_midr, 1f
		b	erratum_\_cpu\()_\_id\()_skip

		1:
	.endif
	_workaround_start \_cpu, \_cve, \_id, \_chosen, 0
.endm

/* keep the _cve argument here so the same #define can be used as the start */
.macro workaround_reset_end _cpu:req, _cve:req, _id:req
	_workaround_end \_cpu, \_id
.endm

.macro workaround_runtime_end _cpu:req, _cve:req, _id:req
	/*
	 * Runtime errata do not have a reset function to call the isb for them
	 * and missing the isb could be very problematic. It is also likely as
	 * they tend to be scattered in generic code.
	 */
	isb
	_workaround_end \_cpu, \_id
.endm

/*******************************************************************************
 * Errata workaround helpers
 ******************************************************************************/
.macro sysreg_bit_set _reg:req, _bit:req, _assert=1
	mrs	x1, \_reg
	orr	x1, x1, #\_bit
	msr	\_reg, x1

#if ENABLE_ASSERTIONS
	/* allow disabling for misbehaving registers */
	.if \_assert
		mrs	x1, \_reg
		tst	x1, #\_bit
		ASM_ASSERT(ne)
	.endif
#endif
.endm

.macro apply_erratum _cpu:req, _cve:req, _id:req, _chosen:req
	.if \_chosen
		mov	x9, x30
		bl	cpu_get_rev_var
		bl	erratum_\_cpu\()_\_id\()_wa
		mov	x30, x9

	.endif
.endm

/*
 * Helpers to select which revisions errata apply to. Don't leave a link
 * register as the cpu_rev_var_*** will call the ret and we can save on one. The
 * _cve argument is again kept to allow the same #defines to be used as above
 */
.macro check_erratum_ls _cpu:req, _cve:req, _id:req, _rev_num:req
	func check_erratum_\_cpu\()_\_id
		mov	x1, #\_rev_num
		b	cpu_rev_var_ls
	endfunc check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_hs _cpu:req, _cve:req, _id:req, _rev_num:req
	func check_erratum_\_cpu\()_\_id
		mov	x1, #\_rev_num
		b	cpu_rev_var_hs
	endfunc check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_range _cpu:req, _cve:req, _id:req, _rev_num_lo:req, _rev_num_hi:req
	func check_erratum_\_cpu\()_\_id
		mov	x1, #\_rev_num_lo
		mov	x2, #\_rev_num_hi
		b	cpu_rev_var_range
	endfunc check_erratum_\_cpu\()_\_id
.endm

/*******************************************************************************
 * CPU reset function wrapper
 ******************************************************************************/
.macro cpu_reset_func_start _cpu:req
	func \_cpu\()_reset_func
		mov	x9, x30
		bl	cpu_get_rev_var
		mov	x10, x0

		/* short circuit the location to avoid searching the list */
		adrp	x11, \_cpu\()_errata_list_start
		add	x11, x11, :lo12:\_cpu\()_errata_list_start
		adrp	x12, \_cpu\()_errata_list_end
		add	x12, x12, :lo12:\_cpu\()_errata_list_end

	errata_begin:
		/* if head catches up with end of list, exit */
		cmp	x11, x12
		b.eq	errata_end

		ldr	x13, [x11, #ERRATA_WA_OFF]
		/* TODO(errata ABI): check mitigated and checker function fields
		 * for 0 */
		ldrb	w14, [x11, #ERRATA_CHOSEN_OFF]

		/* skip if not chosen */
		cbz	x14, 1f
		/* skip if runtime erratum */
		cbz	x13, 1f

		/* put cpu revision in x0 and call workaround */
		mov	x0, x10
		blr	x13
	1:
		add	x11, x11, #ERRATA_ENTRY_SIZE
		b	errata_begin
	errata_end:
.endm

.macro cpu_reset_func_end _cpu:req
		isb
		ret	x9
	endfunc \_cpu\()_reset_func
.endm
#endif /* CPU_MACROS_S */
