#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#Config from make_helpers/defaults.mk
#-------------------------------------------------------------------------------
group_new(NAME default)

#System architecture, define automatic by compiler
group_add(NAME default TYPE CONFIG KEY ARCH VAL aarch64)

#ARM Architecture major and minor versions: 8.0 by default.
group_add(NAME default TYPE CONFIG DEFINE KEY ARM_ARCH_MAJOR VAL 8)
group_add(NAME default TYPE CONFIG DEFINE KEY ARM_ARCH_MINOR VAL 0)

# BL2 image is stored in XIP memory, for now, this option is only supported
# when BL2_AT_EL3 is 1.
group_add(NAME default TYPE CONFIG DEFINE KEY BL2_IN_XIP_MEM VAL 0)

# Do dcache invalidate upon BL2 entry at EL3
group_add(NAME default TYPE DEFINE KEY BL2_INV_DCACHE VAL 1) #TODO: to platform?

# Include FP registers in cpu context
group_add(NAME default TYPE DEFINE KEY CTX_INCLUDE_FPREGS VAL 0)

# Include pointer authentication (ARMv8.3-PAuth) registers in cpu context. This
# must be set to 1 if the platform wants to use this feature in the Secure
# world. It is not needed to use it in the Non-secure world.
group_add(NAME default TYPE DEFINE KEY CTX_INCLUDE_PAUTH_REGS VAL 0)

# Flag to enable Branch Target Identification.
# Internal flag not meant for direct setting.
# Use BRANCH_PROTECTION to enable BTI.
group_add(NAME default TYPE DEFINE KEY ENABLE_BTI VAL 0)

# Build option to enable MPAM for lower ELs
group_add(NAME default TYPE DEFINE KEY ENABLE_MPAM_FOR_LOWER_ELS VAL 0)

# Flag to enable Pointer Authentication.
# Internal flag not meant for direct setting.
# Use BRANCH_PROTECTION to enable PAUTH.
group_add(NAME default TYPE DEFINE KEY ENABLE_PAUTH VAL 0)

# Flag to Enable Position Independant support (PIE)
group_add(NAME default TYPE DEFINE KEY ENABLE_PIE VAL 0)

# Flag to enable runtime instrumentation using PMF
group_add(NAME default TYPE DEFINE KEY ENABLE_RUNTIME_INSTRUMENTATION VAL 0)

# Build option to enable/disable the Statistical Profiling Extensions
group_get(NAME default TYPE CONFIG KEY ARCH RET _arch)
if(_arch STREQUAL aarch64)
	group_add(NAME default TYPE CONFIG DEFINE KEY ENABLE_SPE_FOR_LOWER_ELS VAL 1)
else()
	group_add(NAME default TYPE CONFIG DEFINE KEY ENABLE_SPE_FOR_LOWER_ELS VAL 0)
endif()
unset(_arch)

# For including the Secure Partition Manager
group_add(NAME default TYPE DEFINE KEY ENABLE_SPM VAL 0)

group_add(NAME default TYPE CONFIG KEY ENABLE_STACK_PROTECTOR VAL 0)

# Build flag to treat usage of deprecated platform and framework APIs as error.
group_add(NAME default TYPE DEFINE KEY ERROR_DEPRECATED VAL 0)

# Fault injection support
group_add(NAME default TYPE DEFINE KEY FAULT_INJECTION_SUPPORT VAL 0)

# Hint platform interrupt control layer that Group 0 interrupts are for EL3. By
# default, they are for Secure EL1.
group_add(NAME default TYPE DEFINE KEY GICV2_G0_FOR_EL3 VAL 0)

# Route External Aborts to EL3. Disabled by default; External Aborts are handled
# by lower ELs.
group_add(NAME default TYPE DEFINE KEY HANDLE_EA_EL3_FIRST VAL 0)

# NS timer register save and restore
group_add(NAME default TYPE DEFINE KEY NS_TIMER_SWITCH VAL 0)

# Build PL011 UART driver in minimal generic UART mode
group_add(NAME default TYPE DEFINE KEY PL011_GENERIC_UART VAL 0)

# By default, consider that the platform's reset address is not programmable.
# The platform Makefile is free to override this value.
group_add(NAME default TYPE DEFINE KEY PROGRAMMABLE_RESET_ADDRESS VAL 0)

# Flag used to choose the power state format: Extended State-ID or Original
group_add(NAME default TYPE DEFINE KEY PSCI_EXTENDED_STATE_ID VAL 0)

# Enable RAS support
group_add(NAME default TYPE CONFIG DEFINE KEY RAS_EXTENSION VAL 0)

# Flag to enable exception handling in EL3
group_add(NAME default TYPE CONFIG DEFINE KEY EL3_EXCEPTION_HANDLING VAL 0)

# By default, BL1 acts as the reset handler, not BL31
group_add(NAME default TYPE DEFINE KEY RESET_TO_BL31 VAL 0)

# Flag to introduce an infinite loop in BL1 just before it exits into the next
# image. This is meant to help debugging the post-BL2 phase.
group_add(NAME default TYPE DEFINE KEY SPIN_ON_BL1_EXIT VAL 0)

# Use the SPM based on MM
group_add(NAME default TYPE DEFINE KEY SPM_MM VAL 0) #TODO: mod

# Flags to build TF with Trusted Boot support
group_add(NAME default TYPE DEFINE KEY TRUSTED_BOARD_BOOT VAL 0)

# Build option to choose whether Trusted Firmware uses Coherent memory or not.
#TODO: move to platform.mk
group_add(NAME default TYPE CONFIG DEFINE KEY USE_COHERENT_MEM VAL 1)

# Build option to choose whether Trusted Firmware uses library at ROM
group_add(NAME default TYPE DEFINE KEY USE_ROMLIB VAL 0)

# Use tbbr_oid.h instead of platform_oid.h
group_add(NAME default TYPE DEFINE KEY USE_TBBR_DEFS VAL 1)

# Whether to enable D-Cache early during warm boot. This is usually
# applicable for platforms wherein interconnect programming is not
# required to enable cache coherency after warm reset (eg: single cluster
# platforms).
group_add(NAME default TYPE DEFINE KEY WARMBOOT_ENABLE_DCACHE_EARLY VAL 0)

# Build option to support Secure Interrupt descriptors through fconf
group_add(NAME default TYPE DEFINE CONFIG KEY SEC_INT_DESC_IN_FCONF VAL 0)

#Config from Makefile
#-------------------------------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL Debug)
	group_add(NAME default TYPE CONFIG DEFINE KEY DEBUG VAL 1)
	group_add(NAME default TYPE CONFIG DEFINE KEY BUILD_TYPE VAL debug)
	group_add(NAME default TYPE CONFIG DEFINE KEY LOG_LEVEL VAL 40)
	group_add(NAME default TYPE DEFINE KEY ENABLE_ASSERTIONS VAL 1)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	group_add(NAME default TYPE CONFIG DEFINE KEY DEBUG VAL 0)
	group_add(NAME default TYPE CONFIG DEFINE KEY BUILD_TYPE VAL release)
	group_add(NAME default TYPE CONFIG DEFINE KEY LOG_LEVEL VAL 20)
	group_add(NAME default TYPE DEFINE KEY ENABLE_ASSERTIONS VAL 0)
else()
	message(FATAL_ERROR "Currently only 'Debug' and 'Release' values are supported for 'CMAKE_BUILD_TYPE'.")
endif()

group_add(NAME default TYPE DEFINE KEY XLAT_TABLES_LIB_V2 VAL 1)

#Config from common/backtrace/backtrace.mk
#-------------------------------------------------------------------------------
group_add(NAME default TYPE CONFIG DEFINE KEY ENABLE_BACKTRACE VAL 1)

group_add(NAME default TYPE CONFIG DEFINE KEY SPD VAL none) #TODO: revise this
group_add(NAME default TYPE CONFIG DEFINE KEY SPD_none) #TODO: revise this

#Common include paths
#TODO: better solution?
group_add(NAME default TYPE INCLUDE KEY ${TFA_ROOT_DIR}/include)
group_add(NAME default TYPE INCLUDE KEY ${TFA_ROOT_DIR}/include/arch/@ARCH@)
group_add(NAME default TYPE INCLUDE KEY ${TFA_ROOT_DIR}/include/lib/cpus/@ARCH@)
group_add(NAME default TYPE INCLUDE KEY ${TFA_ROOT_DIR}/include/lib/el3_runtime)
group_add(NAME default TYPE INCLUDE KEY ${TFA_ROOT_DIR}/include/lib/el3_runtime/@ARCH@)
group_add(NAME default TYPE INCLUDE KEY ${TFA_ROOT_DIR}/include/lib/libc)
group_add(NAME default TYPE INCLUDE KEY ${TFA_ROOT_DIR}/include/lib/libc/@ARCH@)
group_add(NAME default TYPE INCLUDE KEY ${TFA_ROOT_DIR}/include/lib/libfdt)

#TODO: AARCH32_INSTRUCTION_SET
#TODO: AARCH32_SP
#TODO: BASE_COMMIT
#TODO: BRANCH_PROTECTION
#TODO: COREBOOT
#TODO: CREATE_KEYS
#TODO: DISABLE_BIN_GENERATION
#TODO: DYN_DISABLE_AUTH
#TODO: ENABLE_STACK_PROTECTOR
#TODO: EL3_EXCEPTION_HANDLING
#TODO: FIP_ALIGN
#TODO: FIP_NAME
#TODO: FWU_FIP_NAME
#TODO: GENERATE_COT
#TODO: ---HANDLE_EA_EL3_FIRST
#TODO: KEY_ALG
#TODO: OVERRIDE_LIBC
#TODO: ---PLAT_${PLAT}
#TODO: SAVE_KEYS
#TODO: SDEI_SUPPORT
#TODO: V
#TODO: ENABLE_SVE_FOR_NS
