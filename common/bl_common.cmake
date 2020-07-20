#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED PROJECT_SOURCE_DIR)
	message(FATAL_ERROR "PROJECT_SOURCE_DIR not defined")
endif()

# Include framework files
include(Common/STGT)

#bl_common
stgt_add_src_param(NAME bl1 bl2 bl31 bl32 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/common/bl_common.c
	${PROJECT_SOURCE_DIR}/common/tf_log.c
	${PROJECT_SOURCE_DIR}/common/@ARCH@/debug.S
	${PROJECT_SOURCE_DIR}/drivers/console/multi_console.c
	${PROJECT_SOURCE_DIR}/lib/@ARCH@/cache_helpers.S
	${PROJECT_SOURCE_DIR}/lib/@ARCH@/misc_helpers.S
	${PROJECT_SOURCE_DIR}/plat/common/plat_bl_common.c
	${PROJECT_SOURCE_DIR}/plat/common/plat_log_common.c
	${PROJECT_SOURCE_DIR}/plat/common/@ARCH@/plat_common.c
	${PROJECT_SOURCE_DIR}/plat/common/@ARCH@/platform_helpers.S
	${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/popcountdi2.c
	${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/popcountsi2.c
	#TODO: armclang, ubsan, stack protector
)

stgt_add_src_cond(NAME bl1 bl2 bl31 bl32 KEY ARCH VAL aarch32 SRC
	${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/arm/aeabi_ldivmod.S
	${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/arm/aeabi_uldivmod.S
	${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/ctzdi2.c
	${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/divdi3.c
	${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/divmoddi4.c
	${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/lshrdi3.c
	${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/udivmoddi4.c
)

stgt_add_src_cond(NAME bl1 bl2 bl31 bl32 KEY ENABLE_BACKTRACE VAL 1 SRC
	${PROJECT_SOURCE_DIR}/common/backtrace/backtrace.c
)
